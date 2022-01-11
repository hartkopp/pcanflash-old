/*
 * pcanflash.c - flash program for PCAN routers
 *
 * Copyright (C) 2021  PEAK System-Technik GmbH
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

#define PCF_MIN_TX_QUEUE 500
#define BUFSZ 512 /* max. known block size */

extern int optind, opterr, optopt;

void print_usage(char *prg)
{
	fprintf(stderr, "\nUsage: %s <options> <interface>\n\n", prg);
	fprintf(stderr, "Options: -f <file.bin>  (binary file to flash)\n");
	fprintf(stderr, "         -i <module_id> (skip question when discovering multiple ids)\n");
	fprintf(stderr, "         -q             (just query modules and quit)\n");
	fprintf(stderr, "         -r             (reset module after flashing)\n");
	fprintf(stderr, "         -d             (dry run - skip erase/write commands)\n");
	fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{
	static uint8_t buf[BUFSZ+2];
	struct ifreq ifr;
	struct sockaddr_can addr;
	static struct can_frame modules[MAX_MODULES];
	struct can_filter rfilter;
	int s; /* CAN_RAW socket */
	static FILE *infile;
	static int query;
	static int do_reset;
	static int dry_run;
	int module_id = NO_MODULE_ID;
	int alternating_xor_flip;
	uint32_t crc_start;
	uint32_t floffset;
	uint32_t blksz;
	int opt, i;
	uint8_t hw_type = 0;
	long foffset;
	int entries;

	while ((opt = getopt(argc, argv, "f:i:qrd?")) != -1) {
		switch (opt) {
		case 'f':
			infile = fopen(optarg, "r");
			if (!infile) {
				perror("infile");
				return 1;
			}
			/* check the file length to fit into 16 MB */
			fseek(infile, 0L, SEEK_END);
			if (ftell(infile) > 0x1000000) {
				printf("binary flash file too long!\n");
				fclose(infile);
				return 1;
			}
			rewind(infile);
			break;

		case 'i':
			module_id = strtoul(optarg, NULL, 10);
			break;

		case 'q':
			query = 1;
			break;

		case 'r':
			do_reset = 1;
			break;

		case 'd':
			dry_run = 1;
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
		return 0;
	}

	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("socket");
		return 1;
	}

	/* set single CAN ID raw filters for RX and TX frames */
	rfilter.can_id	 = CAN_ID & CAN_SFF_MASK;
	rfilter.can_mask = (CAN_SFF_MASK|CAN_EFF_FLAG|CAN_RTR_FLAG);

	setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

	/* copy netdev name for ioctl request */
	strncpy(ifr.ifr_name, argv[optind], sizeof(ifr.ifr_name)-1);

	/* check tx queue length ... */
	if (ioctl(s, SIOCGIFTXQLEN, &ifr) < 0) {
		perror("SIOCGIFTXQLEN");
		return 1;
	}

	/* ... to be at least PCF_MIN_TX_QUEUE CAN frames */
	if (ifr.ifr_qlen < PCF_MIN_TX_QUEUE) {
		fprintf(stderr, "tx queue len %d is too small! Must be at least %d.\n",
			ifr.ifr_qlen, PCF_MIN_TX_QUEUE);
		return 1;
	}

	/* get interface index for bind() */
	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
		perror("SIOCGIFINDEX");
		return 1;
	}
	addr.can_ifindex = ifr.ifr_ifindex;
	addr.can_family = AF_CAN;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}

	entries = query_modules(s, modules);
	if (!entries) {
		fprintf(stderr, "module query failed!\n");
		return 1;
	}

	/* print module list */
	printf("\nfound modules:\n\n");
	for (i = 0; i < MAX_MODULES; i++) {
		if (modules[i].can_id) {
			if (eval_modules(s, i, &modules[i]))
				return 1;
		}
	}

	if (query) {
		printf("\n");
		return 0;
	}

	if (module_id == NO_MODULE_ID) {
		if (entries == 1) {
			/* catch first and only module */
			for (i = 0; i < MAX_MODULES; i++) {
				if (modules[i].can_id) {
					module_id = i;
					break;
				}
			}
		} else {
			printf("\nmultiple modules found - please provide module id : ");
			scanf("%d", &module_id);
			module_id &= MAX_MODULES_MASK;
		}
	}

	if (!(modules[module_id].can_id)) {
		fprintf(stderr, "\nmodule id not found in module list!\n\n");
		exit(1);
	}

	/* restore hw_type of this module_id index from data[7] */
	hw_type = modules[module_id].data[7];

	if (get_hw(hw_type) == NULL) {
		fprintf(stderr, "\nno flash configuration available for hardware type %d!\n\n",
			hw_type);
		exit(1);
	}

	if (check_ch_name(infile, hw_type)) {
		fprintf(stderr, "\nno ch_filename in bin-file for hardware type %d (%s)!\n\n",
			hw_type, get_hw_name(hw_type));
		exit(1);
	}

	/* take default values when not provided by JSON config */
	if (modules[module_id].can_dlc == NO_DATA_LEN) {
		if (has_hw_flags(hw_type, DATA_MODE8))
			modules[module_id].can_dlc = DATA_LEN8;
		else
			modules[module_id].can_dlc = DATA_LEN6;
	}

	blksz = get_max_blocksize(hw_type);
	if ((blksz > BUFSZ) || (blksz < 32)) {
		fprintf(stderr, "\nmax_blocksize %d out of range!\n\n", blksz);
		exit(1);
	}

	printf("\nflashing module id %d with flash transfer data len %d and block size %d\n",
	       module_id, modules[module_id].can_dlc, blksz);

	if (has_hw_flags(hw_type, SWITCH_TO_BOOTLOADER)) { /* PPCAN mode modules */
		printf("\nswitch module into bootloader ... ");
		fflush(stdout);
		switch_to_bootloader(s, module_id);
		sleep(1);
		get_status(s, module_id, NULL);
		printf("done\n");
	}

	printf("\nerasing flash sectors:\n");

	entries = get_num_flashblocks(hw_type);
	if (!(entries)) {
		fprintf(stderr, "no flashblocks found for hardware type %d (%s)!\n",
			hw_type, get_hw_name(hw_type));
		exit(1);
	}
	for (i = 0; i < entries; i++)
		erase_flashblocks(s, dry_run, infile, module_id, hw_type, i);

	printf("\nwriting flash blocks:\n");
	foffset = 0;
	alternating_xor_flip = has_hw_flags(hw_type, FDATA_INVERT);
	crc_start = get_crc_startpos(hw_type);
	floffset = get_flash_offset(hw_type);

	while (1) {

		if (fseek(infile, foffset, SEEK_SET))
			break;

		memset(&buf, 0xFF, blksz);
		fread(buf, 1, blksz, infile);

		for (i = 0; i < blksz; i++) {
			if (buf[i] != EMPTY)
				break;
		}

		/* non-empty block (not all bytes are EMPTY / 0xFFU) */
		if (i != blksz) {

			/* check whether we need to patch the CRC array */
			if ((crc_start) && (crc_start >= foffset) && (crc_start < foffset + blksz))
				write_crc_array(&buf[crc_start - foffset], infile, crc_start);

			/* write non-empty block */
			write_block(s, dry_run, module_id, foffset + floffset, blksz,
				    buf, alternating_xor_flip, modules[module_id].can_dlc);
		}

		if (feof(infile))
			break;

		foffset += blksz;

	} /* while (1) */

	if (has_hw_flags(hw_type, END_PROGRAMMING)) { /* recent hw modules */
		printf("\nend programming ... ");
		fflush(stdout);
		end_programming(s, module_id);
		sleep(1);
		get_status(s, module_id, NULL);
		printf("done\n");
	}

	if (has_hw_flags(hw_type, RESET_AFTER_FLASH) || do_reset) {
		printf("\nreset module ... ");
		fflush(stdout);
		reset_module(s, module_id);
		sleep(1);

		/* a reset which is issued by a command line option
		 * likely leads into starting the application which
		 * does not know about this status message. Therefore
		 * only get the status when this is used in an original
		 * PCAN flashing process, e.g. the PCAN Router Pro
		 */
		if (has_hw_flags(hw_type, RESET_AFTER_FLASH))
			get_status(s, module_id, NULL);

		printf("done\n");
	}

	printf("\ndone.\n\n");

	close(s);
	fclose(infile);

	return 0;
}
