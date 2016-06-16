/*
 * pcanflash.h - flash program for PCAN routers
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

#include <stdint.h>

#define BLKSZ 512
#define NO_MODULE_ID 16
#define CAN_ID 0x7E7

#define SET_STARTADDR   (1 << 0) // 01
#define SET_CHECKSUM    (1 << 1) // 02
#define SET_LENGTH      (1 << 2) // 04
#define SET_ERASE_OK    (1 << 5) // 20
#define SET_CHECKSUM_OK (1 << 6) // 40
#define SET_VERIFY_OK   (1 << 7) // 80

typedef struct {
	uint32_t	address;
	uint32_t	len;
	uint16_t	crc;
} __attribute__((packed)) block_t;

typedef struct {
	uint8_t		str[16];
	uint16_t	version;
	uint16_t	day;
	uint16_t	month;
	uint32_t	year;
	uint16_t	dummy[3];
	uint16_t	mode;  
	uint16_t	count;
	block_t		block[1];
} __attribute__((packed)) crc_array_t;

