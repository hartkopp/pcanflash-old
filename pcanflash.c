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
	fprintf(stderr, "Options:              -f <file.bin>\n");
	fprintf(stderr, "                      -i <module_id>\n");
	fprintf(stderr, "                      -v (increase verbosity level)\n");
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
	static int verbose;
	int module_id = NO_MODULE_ID;
	int alternating_xor_flip;
	uint32_t crc_start;
	int opt, i;
	uint8_t hw_type = 0;
	long foffset;
	int entries;
	crc_array_t *ca;

	while ((opt = getopt(argc, argv, "f:i:v?")) != -1) {
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

		case 'v':
			verbose++;
			break;

		case '?':
		default:
			print_usage(basename(argv[0]));
			return 1;
			break;
		}
	}

        if ((argc - optind) != 1 || (!infile)) {
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
	printf("\nfound modules:\n");
	for (i = 0; i < 16; i++) {
		if (modules[i].can_id) {
			struct can_frame cf;

			printf("module id %02d - date %02X.%02X.20%02X bootloader v%d.%d\n",
			       i, modules[i].data[3], modules[i].data[4], modules[i].data[5],
			       modules[i].data[6] >> 5, modules[i].data[6] & 0x1F);

			get_status(s, i, &cf);
			hw_type = cf.data[3];
			printf("             - hardware %d (%s) flash type %d (%s)\n",
			       hw_type, hw_name(hw_type), cf.data[4], flash_name(cf.data[4]));
		}
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
			printf("multiple modules found - please provide module id : ");
			scanf("%d", &module_id);
			module_id &= 0xF;
		}
	}

	if (!(modules[module_id].can_id)) {
		fprintf(stderr, "module id not found in module list!\n");
		exit(1);
	}

	printf("\nflashing module id : %d\n", module_id);

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
	alternating_xor_flip = get_hw_xor_flip(hw_type);
	crc_start = crc_startpos(hw_type);

	while (1) {

		memset(&buf, 0xFF, sizeof(buf));
		fseek(infile, foffset, SEEK_SET);
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

				printf("str=%s ver=0x%X D/M/Y=%d/%d/%d mode=%d count=%d\n",
				       ca->str, ca->version, ca->day, ca->month, ca->year,
				       ca->mode, ca->count);
				printf("block[0] .address=0x%X  .len=0x%X  .crc=0x%X\n",
				       ca->block[0].address, ca->block[0].len, ca->block[0].crc);

				ca->block[0].crc = calc_crc16(infile, ca->block[0].address, ca->block[0].len);

				printf("block[0] .address=0x%X  .len=0x%X  .crc=0x%X\n",
				       ca->block[0].address, ca->block[0].len, ca->block[0].crc);
			}

			/* write non-empty block */
			write_block(s, module_id, foffset, BLKSZ, buf, alternating_xor_flip);
		}

		if (feof(infile))
			break;

		foffset += BLKSZ;

	} /* while (1) */

	printf("\ndone.\n\n");

	close(s);
	fclose(infile);

	return 0;
}
