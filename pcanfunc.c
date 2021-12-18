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
#include "crc16.h"

#define JSON_BUF_LEN 8000

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

			if ((frame.data[0] & 0xC0 != 0xC0) ||
			    (frame.data[2] != 0x06) ||
			    (frame.can_dlc != 8))
			{
				fprintf(stderr, "received wrong module query!\n");
				exit(1);
			}
			my_id = frame.data[1] & MAX_MODULES_MASK;

			if ((modules + my_id)->can_id)
			{
				fprintf(stderr, "received second module with ID %d!\n", my_id);
				exit(1);
			}
			frame.can_dlc = NO_DATA_LEN; /* prepare data mode storage */
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
	frame.data[3] = CAN2FLASH_STATE_REQUEST;
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

/* simple JSON parsing for relevant content */

#define J_HWTYPE "\"hwType\""
#define J_BOOTLOADER "\"bootloader\""
#define J_FIRMWARE "\"firmware\""
#define J_HARDWARE "\"hardware\""
#define J_DATAMODE "\"dataMode\""
#define J_CANBERESET "\"canBeReset\""

char *findjsonstring(char *buf, const char *jsontag)
{
	char *ptr, *resultstr;

	ptr = strstr(buf, jsontag);
	if (ptr == NULL)
		return NULL;

	/* hop behind the JSON tag */
	ptr += strlen(jsontag);

	/* point to data content */
	ptr = strchr(ptr, '"');
	if (ptr == NULL)
		return NULL;

	ptr++;
	resultstr = ptr;

	/* terminate string */
	ptr = strchr(ptr, '"');
	if (ptr == NULL)
		return NULL;

	*ptr = 0;

	return resultstr;
}

void restorejsonstring(char **ptr)
{
	/* restore trailing double quote for next query */
	*ptr += strlen(*ptr);
	**ptr = '"';
}

uint8_t get_json_config(int s, uint8_t module_id, struct can_frame *modules, struct can_frame *cf)
{
	struct can_frame frame;
	fd_set rdfs;
	struct timeval tv;

	char buf[JSON_BUF_LEN];
	char *ptr;
	unsigned int hwType;

	unsigned char sn = 0; /* JSON PDU counter */
	unsigned char rxsn; /* received JSON PDU counter */
	unsigned int bufptr = 0;
	int ret;

	init_set_cmd(&frame);
	frame.data[2] = module_id;
	frame.data[3] = CAN2FLASH_GET_JSON_DESCRIPTOR;
	frame.data[4] = 0x03; /* 1000 us, high byte */
	frame.data[5] = 0xE8; /* 1000 us, low byte */
	frame.data[6] = 0;

	if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		perror("write");
		exit(1);
	}

	FD_ZERO(&rdfs);
	FD_SET(s, &rdfs);
	tv.tv_sec = 3; /* 3s timeout */
	tv.tv_usec = 0;

json_read_loop:

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

		if ((frame.data[0] != 0x7F) || (frame.data[1] != 0xFF)) {
			fprintf(stderr, "wrong header in in JSON reply string!\n");
			exit(1);
		}

		rxsn = frame.data[2];
		if (rxsn == 0x00) {

			/* start sequence */
			memset(buf, 0, sizeof(buf));
			memcpy(buf, &frame.data[3], 5);
			bufptr = 5;
			sn = 0;

		} else if ((rxsn == 0xFF) || (rxsn == sn + 1)) {

			memcpy(&buf[bufptr], &frame.data[3], 5);
			bufptr += 5;
			sn = rxsn;

			/* rxsn sequence is .. 0xFD 0xFE 0x01 0x02 .. */
			if (sn == 0xFE)
				sn = 0;

			/* ensure buffer size and trailing zero */
			if (bufptr >= (JSON_BUF_LEN - 6)) {
				fprintf(stderr, "JSON buffer length overflow!\n");
				exit(1);
			}
		} else {
			fprintf(stderr, "JSON reception error!\n");
			exit(1);
		}

		if (rxsn == 0xFF) {
			/* we are done */

			//printf("JSON string (len %ld):\n%s\n", strlen(buf), buf);

			printf("module id %02d (ppcan hw id %d)\n",
			       module_id,
			       ((modules->data[0] << 2) | (modules->data[1] >> 6)) & 0xFF);

			ptr = findjsonstring(buf, J_BOOTLOADER);
			if (ptr) {
				printf(" - bootloader %s\n", ptr);
				restorejsonstring(&ptr);
			}

			ptr = findjsonstring(buf, J_FIRMWARE);
			if (ptr) {
				printf(" - firmware %s\n", ptr);
				restorejsonstring(&ptr);
			}

			ptr = findjsonstring(buf, J_HWTYPE);
			if (ptr) {
				if (sscanf(ptr, "%d", &hwType) == 1) {
					hwType &= 0xFF;
					cf->data[3] = hwType;
					cf->data[4] = hwType;

					printf(" - hardware %d (%s) flash type %d (%s)\n",
					       cf->data[3], get_hw_name(cf->data[3]),
					       cf->data[4], get_flash_name(cf->data[4]));

				} else {
					fprintf(stderr, "JSON buffer parse error (%s)!\n", J_HWTYPE);
					exit(1);
				}
				restorejsonstring(&ptr);
			}

			ptr = findjsonstring(buf, J_DATAMODE);
			if (ptr) {
				if (modules->can_dlc != NO_DATA_LEN) {
					fprintf(stderr, "JSON datamode not empty!\n");
					exit(1);
				}

				if (*ptr == '0')
					modules->can_dlc = DATA_LEN6;
				else if (*ptr == '1')
					modules->can_dlc = DATA_LEN8;
				else {
					fprintf(stderr, "JSON unknown datamode '%c'!\n", *ptr);
					exit(1);
				}
				printf(" - datamode %c => flash transfer data len %d\n", *ptr, modules->can_dlc);

				restorejsonstring(&ptr);
			}

			return 0;
		}

		goto json_read_loop;
	}

	fprintf(stderr, "timeout in get_status process!\n");
	exit(1);
}

int eval_modules(int s, int module_id, struct can_frame *modules)
{
	struct can_frame cf;

	/* get status for this found module */
	get_status(s, module_id, &cf);

	/* hardware type or flash type is 250 => get info via JSON config string */
	if ((cf.data[3] == 250) || (cf.data[4] == 250)) {
		if (get_json_config(s, module_id, modules, &cf)) {
			fprintf(stderr, "\nError reading the JSON configuration string!\n\n");
			exit(1);
		}
	} else {
		printf("module id %02d (ppcan hw id %d)\n",
		       module_id,
		       ((modules->data[0] << 2) | (modules->data[1] >> 6)) & 0xFF);

		printf(" - date %02X.%02X.20%02X bootloader v%d.%d\n",
		       modules->data[3], modules->data[4], modules->data[5],
		       modules->data[6] >> 5, modules->data[6] & 0x1F);

		printf(" - hardware %d (%s) flash type %d (%s)\n",
		       cf.data[3], get_hw_name(cf.data[3]),
		       cf.data[4], get_flash_name(cf.data[4]));
	}
	/* check if hardware fits to known flash id type */
	if (check_flash_id_type(cf.data[3], cf.data[4])) {
		fprintf(stderr, "\nFlash ID type does not match the hardware ID!\n\n");
		return 1;
	}

	/* store hw_type for this module_id index in data[7] */
	modules->data[7] = cf.data[3];

	return 0;
}

void write_crc_array(uint8_t *buf, FILE *infile, uint32_t crc_start)
{
	crc_array_t *ca = (crc_array_t *)buf;
	int i;

	if (strcmp((const char *)ca->str, CRC_IDENT_STRING)) {
		fprintf(stderr, " no CRC Ident string found - omit patching of CRC value.\n");
		return;
	}

	printf(" CRC array ver=0x%X D/M/Y=%d/%d/%d mode=%d found at 0x%X\n",
	       ca->version, ca->day, ca->month, ca->year, ca->mode, crc_start);

	if ((ca->mode == 1) || (ca->mode == 3) || (ca->mode == 4)) {
		for (i = 0; i < ca->count; i++) {
			ca->block[i].crc = calc_crc16(infile, ca->block[i].address, ca->block[i].len);
			printf(" CRC block[%d] .address=0x%X  .len=0x%X	 .crc=0x%X\n",
			       i, ca->block[i].address, ca->block[i].len, ca->block[i].crc);
		}
	} else
		printf(" CRC array mode=%d is not supported - omit patching of CRC value.\n", ca->mode);
}

void write_block(int s, int dry_run, uint8_t module_id, uint32_t offset, uint32_t blksz, uint8_t *buf, uint32_t alternating_xor_flip, uint8_t ftd_len)
{
	struct can_frame frame;
	int i, j, xor_flip;
	uint8_t status;
	uint16_t csum;

	for (i = 0, csum = 0; i < blksz; i++)
		csum = (csum + *(buf +i)) & 0xFFFFU;

	printf ("writing non empty block at offset 0x%X with csum 0x%04X\n",
		(unsigned int)offset, (unsigned int)csum);

	set_startaddress(s, module_id, offset);
	status = get_status(s, module_id, NULL);
	if ((status & SET_STARTADDR) != (SET_STARTADDR)) {
		fprintf(stderr, "flash1 - wrong status %02X!\n", status);
		exit(1);
	}
	
	set_blocksize(s, module_id, blksz);
	status = get_status(s, module_id, NULL);
	if ((status & (SET_STARTADDR | SET_LENGTH)) != (SET_STARTADDR | SET_LENGTH)) {
		fprintf(stderr, "flash2 - wrong status %02X!\n", status);
		exit(1);
	}

	frame.can_id = CAN_ID;
	frame.can_dlc = 8;
	xor_flip = 0;

	/* prepare frame for DATA_LEN6 */
	if (ftd_len == DATA_LEN6) {
		frame.data[0] = 0x7F;
		frame.data[1] = 0xFF;
	}

	for (i = 0; i < blksz; i += ftd_len, xor_flip ^= 1) {

		uint8_t len = ftd_len;

		if ((ftd_len == DATA_LEN6) && (i + ftd_len >= blksz)) {
			/* last frame for DATA_LEN6 */
			memset(&frame.data[2], 0, DATA_LEN6);
			len = blksz - i;
		}

		for (j = 0; j < len; j++)
			frame.data[j + (8 - ftd_len)] = *(buf + i + j);

		if ((xor_flip) && (alternating_xor_flip)) {
			for (j = 0; j < len; j++)
				frame.data[j + (8 - ftd_len)] ^= 0xFF;
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
	
	if (!dry_run) {
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
}

void erase_block(int s, int dry_run, uint8_t module_id, uint32_t startaddr, uint32_t blksz)
{
	uint8_t status;

	printf ("erasing block at startaddr 0x%06X with block size 0x%06X\n",
		(unsigned int)startaddr, (unsigned int)blksz);

	set_startaddress(s, module_id, startaddr);
	status = get_status(s, module_id, NULL);
	if ((!dry_run) && ((status & SET_STARTADDR) != SET_STARTADDR)) {
		fprintf(stderr, "erase1 - wrong status %02X!\n", status);
		exit(1);
	}
	
	set_blocksize(s, module_id, blksz);
	status = get_status(s, module_id, NULL);
	if ((!dry_run) && ((status & (SET_STARTADDR | SET_LENGTH)) != (SET_STARTADDR | SET_LENGTH))) {
		fprintf(stderr, "erase2 - wrong status %02X!\n", status);
		exit(1);
	}
	
	if (!dry_run) {
		erase_sector(s, module_id);
		status = get_status(s, module_id, NULL);
		if ((status & SET_ERASE_OK) != SET_ERASE_OK) {
			fprintf(stderr, "erase3 - wrong status %02X!\n", status);
			exit(1);
		}
	}
}

void erase_flashblocks(int s, int dry_run, FILE *infile, uint8_t module_id, uint8_t hw_type, int index)
{
	const fblock_t *fblock;
	uint8_t data;
	int i;

	const hw_t *hwt = get_hw(hw_type);
	const uint32_t flash_offset = get_flash_offset(hw_type);

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

	/* check for wrong flash_offset configuration */
	if (fblock->start < flash_offset) {
		fprintf(stderr, "bad flashblock offset 0x%X for flashblock "
			"start at 0x%X found for hardware type %d (%s)!\n",
			flash_offset, fblock->start,
			hw_type, get_hw_name(hw_type));
		exit(1);
	}

	/* check block in bin-file */
	if (fseek(infile, fblock->start - flash_offset, SEEK_SET))
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

	erase_block(s, dry_run, module_id, fblock->start, fblock->len);
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

		if (!strncmp(buf, hwt->ch_file, strlen(hwt->ch_file)))
			return 0; /* match */

		/* no match -> rewind back behind the 'P' */
		if (fseek(infile, ftell(infile) - (HW_NAME_MAX_LEN - 1), SEEK_SET))
			return 1;
	}
}
