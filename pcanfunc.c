/*
 * pcanfunc.c - flash program for PCAN routers
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

#include <sys/time.h>
#include <sys/types.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "pcanflash.h"
#include "pcanhw.h"

int query_modules(int s, struct can_frame *modules)
{
	int entries = 0;
	int have_rx = 1;
	int my_id, ret;
	fd_set rdfs;
	struct timeval tv;
	struct can_frame frame;

	/* send module query request */
	memset(&frame, 0, sizeof(struct can_frame));

	frame.can_id = CAN_ID;
	frame.can_dlc = 3;
	frame.data[0] = 0x80;
	frame.data[1] = 0x00;
	frame.data[2] = 0x06;

	if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		perror("write");
		exit(1);
	}

	while (have_rx) {

		have_rx = 0;
		FD_ZERO(&rdfs);
		FD_SET(s, &rdfs);
		tv.tv_sec = 1; /* 1s timeout */
		tv.tv_usec = 0;

		ret = select(s+1, &rdfs, NULL, NULL, &tv);
		if (ret < 0) {
			perror("select");
			exit(1);
		}

		if (FD_ISSET(s, &rdfs)) {
			have_rx = 1;
			ret = read(s, &frame, sizeof(struct can_frame));
			if (ret < 0) {
				perror("read");
				exit(1);
			}

			if ((frame.data[0] & 0xDF != 0xC6) ||
			    ((frame.data[1] & 0xF0) != 0x40) ||
			    (frame.data[2] != 0x06) ||
			    (frame.can_dlc != 8))
			{
				fprintf(stderr, "received wrong module query!\n");
				exit(1);
			}
			my_id = frame.data[1] & 0xF;

			if ((modules + my_id)->can_id)
			{
				fprintf(stderr, "received second module with ID %d!\n", my_id);
				exit(1);
			}
			memcpy(modules + my_id, &frame, sizeof(struct can_frame));
			entries++;
		}
	}

	return entries;
}

void init_set_cmd(struct can_frame *frame)
{
	frame->can_id = CAN_ID;
	frame->can_dlc = 7;
	frame->data[0] = 0x7F;
	frame->data[1] = 0xFF;
	frame->data[7] = 0x00;
}

void set_startaddress(int s, uint8_t module_id, uint32_t addr)
{
	struct can_frame frame;

	init_set_cmd(&frame);
	frame.data[2] = module_id;
	frame.data[3] = CAN2FLASH_SET_STARTADDRESS;
	frame.data[4] = (addr >> 16) & 0xFF;
	frame.data[5] = (addr >> 8) & 0xFF;
	frame.data[6] = addr & 0xFF;

	if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		perror("write");
		exit(1);
	}
}

void set_blocksize(int s, uint8_t module_id, uint32_t size)
{
	struct can_frame frame;

	init_set_cmd(&frame);
	frame.data[2] = module_id;
	frame.data[3] = CAN2FLASH_SET_BLOCKSIZE;
	frame.data[4] = (size >> 16) & 0xFF;
	frame.data[5] = (size >> 8) & 0xFF;
	frame.data[6] = size & 0xFF;

	if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		perror("write");
		exit(1);
	}
}

void set_checksum(int s, uint8_t module_id, uint16_t csum)
{
	struct can_frame frame;

	init_set_cmd(&frame);
	frame.data[2] = module_id;
	frame.data[3] = CAN2FLASH_SET_CHECKSUM;
	frame.data[4] = (csum >> 8) & 0xFF;
	frame.data[5] = csum & 0xFF;
	frame.data[6] = 0;
    
	if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		perror("write");
		exit(1);
	}
}

void erase_sector(int s, uint8_t module_id)
{
	struct can_frame frame;

	init_set_cmd(&frame);
	frame.data[2] = module_id;
	frame.data[3] = CAN2FLASH_ERASE_SECTOR;
	frame.data[4] = 0x55;
	frame.data[5] = 0;
	frame.data[6] = 0;
    
	if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		perror("write");
		exit(1);
	}
}

void start_programming(int s, uint8_t module_id)
{
	struct can_frame frame;

	init_set_cmd(&frame);
	frame.data[2] = module_id;
	frame.data[3] = CAN2FLASH_START_PROGRAMMING;
	frame.data[4] = 0x55;
	frame.data[5] = 0;
	frame.data[6] = 0;
    
	if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		perror("write");
		exit(1);
	}
}

void verify(int s, uint8_t module_id)
{
	struct can_frame frame;

	init_set_cmd(&frame);
	frame.data[2] = module_id;
	frame.data[3] = CAN2FLASH_VERIFY;
	frame.data[4] = 0;
	frame.data[5] = 0;
	frame.data[6] = 0;
    
	if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		perror("write");
		exit(1);
	}
}

void switch_to_bootloader(int s, uint8_t module_id)
{
	struct can_frame frame;

	init_set_cmd(&frame);
	frame.data[2] = module_id;
	frame.data[3] = CAN2FLASH_SWITCH_TO_BOOTLOADER;
	frame.data[4] = 0x55;
	frame.data[5] = 0;
	frame.data[6] = 0;

	if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		perror("write");
		exit(1);
	}
}

void reset_module(int s, uint8_t module_id)
{
	struct can_frame frame;

	init_set_cmd(&frame);
	frame.data[2] = module_id;
	frame.data[3] = CAN2FLASH_RESET_REQUEST;
	frame.data[4] = 0x55;
	frame.data[5] = 0;
	frame.data[6] = 0;

	if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		perror("write");
		exit(1);
	}
}

void end_programming(int s, uint8_t module_id)
{
	struct can_frame frame;

	init_set_cmd(&frame);
	frame.data[2] = module_id;
	frame.data[3] = CAN2FLASH_END;
	frame.data[4] = 0;
	frame.data[5] = 0;
	frame.data[6] = 0;

	if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		perror("write");
		exit(1);
	}
}

uint8_t get_status(int s, uint8_t module_id, struct can_frame *cf)
{
	struct can_frame frame;
	fd_set rdfs;
	struct timeval tv;
	int ret;

	init_set_cmd(&frame);
	frame.data[2] = module_id;
	frame.data[3] = 0;
	frame.data[4] = 0;
	frame.data[5] = 0;
	frame.data[6] = 0;
    
	if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		perror("write");
		exit(1);
	}

	FD_ZERO(&rdfs);
	FD_SET(s, &rdfs);
	tv.tv_sec = 3; /* 3s timeout */
	tv.tv_usec = 0;

	ret = select(s+1, &rdfs, NULL, NULL, &tv);
	if (ret < 0) {
		perror("select");
		exit(1);
	}

	if (FD_ISSET(s, &rdfs)) {
		ret = read(s, &frame, sizeof(struct can_frame));
		if (ret < 0) {
			perror("read");
			exit(1);
		}

		if (cf)
			memcpy(cf, &frame, sizeof(struct can_frame));

		return frame.data[5];
	}

	fprintf(stderr, "timeout in get_status process!\n");
	exit(1);
}

void write_block(int s, uint8_t module_id, uint32_t offset, uint32_t blksz, uint8_t *buf, uint32_t alternating_xor_flip)
{
	struct can_frame frame;
	int i, j;
	uint8_t status;
	uint16_t csum;

	for (i = 0, csum = 0; i < BLKSZ; i++)
		csum = (csum + *(buf +i)) & 0xFFFFU;

	printf ("writing non empty block at offset 0x%X with csum 0x%04X\n",
		(unsigned int)offset, (unsigned int)csum);

	set_startaddress(s, module_id, offset);
	status = get_status(s, module_id, NULL);
	if ((status & SET_STARTADDR) != (SET_STARTADDR)) {
		fprintf(stderr, "flash1 - wrong status %02X!\n", status);
		exit(1);
	}
	
	set_blocksize(s, module_id, BLKSZ);
	status = get_status(s, module_id, NULL);
	if ((status & (SET_STARTADDR | SET_LENGTH)) != (SET_STARTADDR | SET_LENGTH)) {
		fprintf(stderr, "flash2 - wrong status %02X!\n", status);
		exit(1);
	}

	frame.can_id = CAN_ID;
	frame.can_dlc = 8;

	for (i = 0; i < BLKSZ; i += 8) {

		for (j = 0; j < 8; j++)
			frame.data[j] = *(buf + i + j);

		if ((i & 8) && (alternating_xor_flip)) {
			for (j = 0; j < 8; j++)
				frame.data[j] ^= 0xFF;
		}

		if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
			perror("write");
			exit(1);
		}
	}

	status = get_status(s, module_id, NULL);
	if ((status & (SET_STARTADDR | SET_LENGTH)) != (SET_STARTADDR | SET_LENGTH)) {
		fprintf(stderr, "flash3 - wrong status %02X!\n", status);
		exit(1);
	}
	
	set_checksum(s, module_id, csum);
	status = get_status(s, module_id, NULL);
	if (status != (SET_CHECKSUM_OK | SET_STARTADDR | SET_LENGTH | SET_CHECKSUM)) {
		fprintf(stderr, "flash4 - wrong status %02X!\n", status);
		fprintf(stderr, "Please check CAN netdevice tx-queue-len to avoid block data loss.\n");
		exit(1);
	}
	
	start_programming(s, module_id);
	status = get_status(s, module_id, NULL);
	if (status != (SET_CHECKSUM_OK)) {
		fprintf(stderr, "flash5 - wrong status %02X!\n", status);
		exit(1);
	}
	
	verify(s, module_id);
	status = get_status(s, module_id, NULL);
	if (status != (SET_CHECKSUM_OK | SET_VERIFY_OK)) {
		fprintf(stderr, "flash6 - wrong status %02X!\n", status);
		exit(1);
	}
}

void erase_block(int s, uint8_t module_id, uint32_t startaddr, uint32_t blksz)
{
	uint8_t status;

	printf ("erasing block at startaddr 0x%06X with block size 0x%06X\n",
		(unsigned int)startaddr, (unsigned int)blksz);

	set_startaddress(s, module_id, startaddr);
	status = get_status(s, module_id, NULL);
	if (status != (SET_CHECKSUM_OK | SET_STARTADDR)) {
		fprintf(stderr, "erase1 - wrong status %02X!\n", status);
		exit(1);
	}
	
	set_blocksize(s, module_id, blksz);
	status = get_status(s, module_id, NULL);
	if (status != (SET_CHECKSUM_OK | SET_STARTADDR | SET_LENGTH)) {
		fprintf(stderr, "erase2 - wrong status %02X!\n", status);
		exit(1);
	}
	
	erase_sector(s, module_id);
	status = get_status(s, module_id, NULL);
	if (status != (SET_CHECKSUM_OK | SET_ERASE_OK)) {
		fprintf(stderr, "erase3 - wrong status %02X!\n", status);
		exit(1);
	}
}

void erase_flashblocks(int s, FILE *infile, uint8_t module_id, uint8_t hw_type, int index)
{
	const fblock_t *fblock;
	uint8_t data;
	int i;

	const hw_t *hwt = get_hw(hw_type);

	if (hwt)
		fblock = &hwt->flashblocks[index];
	else {
		fprintf(stderr, "bad flashblocks entry found for hardware type %d (%s)!\n",
			hw_type, get_hw_name(hw_type));
		exit(1);
	}

	/* skip handling of this flash block? */
	if (fblock->skipped)
		return;

	/* check block in bin-file */
	if (fseek(infile, fblock->start, SEEK_SET))
		return;

	for (i = 0; i < fblock->len; i++) {
		if (fread(&data, 1, 1, infile) != 1) {
			/* file ended but was empty so far -> no action */
			return;
		}
		if (data != EMPTY)
			break;
	}

	/* empty block (all bytes are EMPTY / 0xFFU) -> no action */
	if (i == fblock->len)
		return;

	erase_block(s, module_id, fblock->start, fblock->len);
}

int check_ch_name(FILE *infile, uint8_t hw_type)
{
	const hw_t *hwt = get_hw(hw_type);
	char buf[HW_NAME_MAX_LEN + 2];

	memset(buf, 0, sizeof(buf));

	if (!hwt)
		return 1;

	rewind(infile);

	while (1) {

		if (fread(buf, 1, 1, infile) != 1)
			return 1;

		/* PCAN devices always start with 'P' */
		if (buf[0] != 'P')
			continue;

		if (fread(&buf[1], 1, HW_NAME_MAX_LEN - 1, infile) != HW_NAME_MAX_LEN - 1)
			return 1;

		if (!strncmp(buf, hwt->ch_file, HW_NAME_MAX_LEN))
			return 0; /* match */

		/* no match -> rewind back behind the 'P' */
		if (fseek(infile, ftell(infile) - (HW_NAME_MAX_LEN - 1), SEEK_SET))
			return 1;
	}
}
