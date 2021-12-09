/*
 * pcanhw.h - flash program for PCAN routers
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
#include <stdint.h>

/* empty flash content -> all bits are set */
#define EMPTY 0xFFU

typedef struct {
	uint32_t start;
	uint32_t len;
} fblock_t;

#define HW_NAME_MAX_LEN 24
#define UNKNOWN_FLASH_ID 0

typedef struct {
	const char name[HW_NAME_MAX_LEN];
	const char ch_file[HW_NAME_MAX_LEN];
	const uint32_t flags;
	const uint32_t crc_startpos;
	const uint32_t flash_offset;
	const uint8_t flash_id_type;
	const int num_flashblocks;
	const fblock_t *flashblocks;
} hw_t;

/* hardware type specific flags */

#define FDATA_INVERT		(1<<0)
#define SWITCH_TO_BOOTLOADER	(1<<1)
#define RESET_AFTER_FLASH	(1<<2)
#define END_PROGRAMMING		(1<<3)

const hw_t *get_hw(uint8_t hw_type);
uint32_t get_crc_startpos(uint8_t hw_type);
uint32_t get_flash_offset(uint8_t hw_type);
uint32_t has_hw_flags(uint8_t hw_type, const uint32_t flags);
const char *get_hw_name(uint8_t hw_type);
const char *get_flash_name(uint8_t flash_type);
int get_num_flashblocks(uint8_t hw_type);
int check_flash_id_type(uint8_t hw_type, uint8_t flash_id_type);
