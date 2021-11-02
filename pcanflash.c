/*
 * pcanflash.c - flash program for PCAN routers
 *
 * Copyright (C) 2016  PEAK System-Technik GmbH
 *
 * linux@peak-system.com
 * www.peak-system.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Author: Oliver Hartkopp (socketcan@hartkopp.net)
 * Maintainer(s): Stephane Grosjean (s.grosjean@peak-system.com)
 *
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "pcanflash.h"
#include "pcanfunc.h"
#include "pcanhw.h"
#include "crc16.h"

extern int optind, opterr, optopt;


void print_usage(char *prg)
{
	fprintf(stderr, "\nUsage: %s <options> <interface>\n\n", prg);
	fprintf(stderr, "Options: -f <file.bin>  (binary file to flash)\n");
	fprintf(stderr, "         -i <module_id> (skip question when discovering multiple ids)\n");
	fprintf(stderr, "         -q             (just query modules and quit)\n");
	fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{
	static uint8_t buf[BLKSZ+2];
	struct sockaddr_can addr;
	static struct can_frame modules[16];
        struct can_filter rfilter;
	int s; /* CAN_RAW socket */
	static FILE *infile;
	static int query;
	int module_id = NO_MODULE_ID;
	int alternating_xor_flip;
	uint32_t crc_start;
	int opt, i;
	uint8_t hw_type = 0;
	long foffset;
	int entries;
	crc_array_t *ca;

	while ((opt = getopt(argc, argv, "f:i:q?")) != -1) {
		switch (opt) {
		case 'f':
			infile = fopen(optarg, "r");
			if (!infile) {
				perror("infile");
				return 1;
			}
			break;

		case 'i':
			module_id = strtoul(optarg, NULL, 10);
			break;

		case 'q':
			query = 1;
			break;

		case '?':
		default:
			print_usage(basename(argv[0]));
			return 1;
			break;
		}
	}

        if ((argc - optind) != 1 || (infile && query) || (!(infile || query))) {
                print_usage(basename(argv[0]));
                exit(0);
        }

        if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
                perror("socket");
                return 1;
        }

        /* set single CAN ID raw filters for RX and TX frames */
	rfilter.can_id   = CAN_ID & CAN_SFF_MASK;
	rfilter.can_mask = (CAN_SFF_MASK|CAN_EFF_FLAG|CAN_RTR_FLAG);

        setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

        addr.can_family = AF_CAN;
        addr.can_ifindex = if_nametoindex(argv[optind]);

        if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("bind");
                return 1;
        }

	entries = query_modules(s, modules);
	if (!entries) {
		fprintf(stderr, "module query failed!\n");
		exit(1);
	}

	/* print module list */
	printf("\nfound modules:\n\n");
	for (i = 0; i < 16; i++) {
		if (modules[i].can_id) {
			struct can_frame cf;

			printf("module id %02d - date %02X.%02X.20%02X bootloader v%d.%d\n",
			       i, modules[i].data[3], modules[i].data[4], modules[i].data[5],
			       modules[i].data[6] >> 5, modules[i].data[6] & 0x1F);

			get_status(s, i, &cf);
			/* store hw_type for this module_id index in data[7] */
			modules[i].data[7] = cf.data[3];
			printf("             - hardware %d (%s) flash type %d (%s)\n",
			       cf.data[3], hw_name(cf.data[3]), cf.data[4], flash_name(cf.data[4]));

			/* check if hardware fits to known flash id type */
			if (check_flash_id_type(cf.data[3], cf.data[4])) {
				fprintf(stderr, "\nFlash ID type does not match the hardware ID!\n\n");
				exit(1);
			}
		}
	}

	if (query) {
		printf("\n");
		return 0;
	}

	if (module_id == NO_MODULE_ID) {
		if (entries == 1) {
			/* catch first and only module */
			for (i = 0; i < 16; i++) {
				if (modules[i].can_id) {
					module_id = i;
					break;
				}
			}
		} else {
			printf("\nmultiple modules found - please provide module id : ");
			scanf("%d", &module_id);
			module_id &= 0xF;
		}
	}

	if (!(modules[module_id].can_id)) {
		fprintf(stderr, "\nmodule id not found in module list!\n\n");
		exit(1);
	}

	/* restore hw_type of this module_id index from data[7] */
	hw_type = modules[module_id].data[7];

	if (check_ch_name(infile, hw_type)) {
		fprintf(stderr, "\nno ch_filename in bin-file for hardware type %d (%s)!\n\n",
			hw_type, hw_name(hw_type));
		exit(1);
	}

	printf("\nflashing module id : %d\n", module_id);

	if (hw_flags(hw_type, SWITCH_TO_BOOTLOADER)) { /* PPCAN mode modules */
		printf("\nswitch module into bootloader ... ");
		fflush(stdout);
		switch_to_bootloader(s, module_id);
		sleep(1);
		get_status(s, module_id, NULL);
		printf("done\n");
	}

	printf("\nerasing flash sectors:\n");

	entries = num_flashblocks(hw_type);
	if (!(entries)) {
		fprintf(stderr, "no flashblocks found for hardware type %d (%s)!\n",
			hw_type, hw_name(hw_type));
		exit(1);
	}
	for (i = 0; i < entries; i++)
		erase_flashblocks(s, infile, module_id, hw_type, i);

	printf("\nwriting flash blocks:\n");
	foffset = 0;
	alternating_xor_flip = hw_flags(hw_type, FDATA_INVERT);
	crc_start = crc_startpos(hw_type);

	while (1) {

		if (fseek(infile, foffset, SEEK_SET))
			break;

		memset(&buf, 0xFF, sizeof(buf));
		fread(buf, 1, BLKSZ, infile);

		for (i = 0; i < BLKSZ; i++) {
			if (buf[i] != EMPTY)
				break;
		}

		/* non-empty block (not all bytes are EMPTY / 0xFFU) */
		if (i != BLKSZ) {

			/* check whether we need to patch the CRC array */
			if ((crc_start) && (crc_start >= foffset) && (crc_start < foffset + BLKSZ)) {

				/* access crc_array */
				ca = (crc_array_t *)&buf[crc_start - foffset];
				if (!strcmp((const char *)ca->str, CRC_IDENT_STRING)) {
					printf(" CRC array ver=0x%X D/M/Y=%d/%d/%d mode=%d found at 0x%X\n",
					       ca->version, ca->day, ca->month, ca->year, ca->mode, crc_start);

					if ((ca->mode == 1) || (ca->mode == 3) || (ca->mode == 4)) {
						for (i = 0; i < ca->count; i++) {
							ca->block[i].crc = calc_crc16(infile, ca->block[i].address, ca->block[i].len);
							printf(" CRC block[%d] .address=0x%X  .len=0x%X  .crc=0x%X\n",
							       i, ca->block[i].address, ca->block[i].len, ca->block[i].crc);
						}
					} else
						printf(" CRC array mode=%d is not supported - omit  patching of CRC value.\n", ca->mode);
				} else
					fprintf(stderr, " no CRC Ident string found - omit  patching of CRC value.\n");
			}

			/* write non-empty block */
			write_block(s, module_id, foffset, BLKSZ, buf, alternating_xor_flip);
		}

		if (feof(infile))
			break;

		foffset += BLKSZ;

	} /* while (1) */

	if (hw_flags(hw_type, END_PROGRAMMING)) { /* recent hw modules */
		printf("\nend programming ... ");
		fflush(stdout);
		end_programming(s, module_id);
		sleep(1);
		get_status(s, module_id, NULL);
		printf("done\n");
	}

	if (hw_flags(hw_type, RESET_AFTER_FLASH)) { /* PPCAN mode modules */
		printf("\nreset module ... ");
		fflush(stdout);
		reset_module(s, module_id);
		sleep(1);
		get_status(s, module_id, NULL);
		printf("done\n");
	}

	printf("\ndone.\n\n");

	close(s);
	fclose(infile);

	return 0;
}
