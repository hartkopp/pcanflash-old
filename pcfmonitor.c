/*
 * pcfmonitor.c - flash program monitor for PCAN routers
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

#define ATTRESET "\33[0m"
#define FGRED    "\33[31m"
#define FGBLUE   "\33[34m"

extern int optind, opterr, optopt;

void print_usage(char *prg)
{
	fprintf(stderr, "\nUsage: %s <options> <interface>\n\n", prg);
	fprintf(stderr, "Options: -c (color)\n");
	fprintf(stderr, "\n");
}

void print_cmd(struct can_frame cf, int color)
{
	if (color)
		printf("%s", FGRED);

	printf("[%d] ", cf.data[2]);

	switch (cf.data[3]) {

	case CAN2FLASH_STATE_REQUEST:
		printf("GetStatus");
		break;

	case CAN2FLASH_SET_STARTADDRESS:
		printf("SetStartAddr 0x%02X%02X%02X", cf.data[4], cf.data[5], cf.data[6]);
		break;

	case CAN2FLASH_SET_BLOCKSIZE:
		printf("SetBlockSize 0x%02X%02X%02X", cf.data[4], cf.data[5], cf.data[6]);
		break;

	case CAN2FLASH_SET_CHECKSUM:
		printf("SetCheckSum 0x%02X%02X", cf.data[4], cf.data[5]);
		break;

	case CAN2FLASH_ERASE_SECTOR:
		if (cf.data[4] == 0x55)
			printf("EraseSector");
		else
			printf("EraseSector (unknown)");
		break;

	case CAN2FLASH_START_PROGRAMMING:
		if (cf.data[4] == 0x55)
			printf("StartProgramming");
		else
			printf("StartProgramming (unknown)");
		break;

	case CAN2FLASH_VERIFY:
		printf("Verify");
		break;

	case CAN2FLASH_SWITCH_TO_BOOTLOADER:
		if (cf.data[4] == 0x55)
			printf("SwitchToBootloader");
		else
			printf("SwitchToBootloader (unknown)");
		break;

	case CAN2FLASH_RESET_REQUEST:
		if (cf.data[4] == 0x55)
			printf("ResetModule");
		else
			printf("ResetModule (unknown)");
		break;

	case CAN2FLASH_END:
		printf("EndProgramming");
		break;

	default:
		printf("(unknown command) 0x%02X", cf.data[3]);
		break;
	}

	printf("\n");

	if (color)
		printf("%s", ATTRESET);

	fflush(stdout);
}

void print_status(struct can_frame cf, int color)
{
	uint8_t status = cf.data[5];

	if (color)
		printf("%s", FGBLUE);

	printf("[%d] (0x%02X) ", cf.data[2], status);

	if (status & SET_STARTADDR)
		printf("SET_STARTADDR ");
	if (status & SET_CHECKSUM)
		printf("SET_CHECKSUM ");
	if (status & SET_LENGTH)
		printf("SET_LENGTH ");
	if (status & SET_ERASE_OK)
		printf("SET_ERASE_OK ");
	if (status & SET_CHECKSUM_OK)
		printf("SET_CHECKSUM_OK ");
	if (status & SET_VERIFY_OK)
		printf("SET_VERIFY_OK ");

	printf("\n");

	if (color)
		printf("%s", ATTRESET);

	fflush(stdout);
}

int main(int argc, char **argv)
{
	int s; /* CAN_RAW socket */
	struct sockaddr_can addr;
        struct can_filter rfilter;
        struct can_frame cf;
	int opt;
	int ret;
	int color = 0;

	while ((opt = getopt(argc, argv, "c?")) != -1) {
		switch (opt) {
		case 'c':
			color = 1;
			break;

		case '?':
		default:
			print_usage(basename(argv[0]));
			return 1;
			break;
		}
	}

        if ((argc - optind) != 1) {
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

	while (1) {
		ret = read(s, &cf, sizeof(struct can_frame));
		if (ret != sizeof(struct can_frame)) {
			perror("read");
			exit(1);
		}

		if ((cf.can_dlc < 6) || (cf.can_dlc > 7))
			continue;

		if ((cf.data[0] != 0x7F) || (cf.data[1] != 0xFF))
			continue;

		if (cf.can_dlc == 7)
			print_cmd(cf, color);
		else
			print_status(cf, color);
	}

	close(s);

	return 0;
}
