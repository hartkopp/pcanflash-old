/*
 * pcanhw.c - flash program for PCAN routers
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "pcanhw.h"
#include "pcanfunc.h"

/* flash block layouts */
#include "flash_types.h"

/* hardware type descriptions */
#include "hardware_types.h"

const hw_t *get_hw(uint8_t hw_type)
{
	if (hw_type > (sizeof(hwtab)/sizeof(*hwtab)) - 1)
		return NULL; /* unknown */

	return hwtab[hw_type];
}

uint32_t get_crc_startpos(uint8_t hw_type)
{
	const hw_t *hwt = get_hw(hw_type);

	if (hwt)
		return hwt->crc_startpos;

	return 0; /* disabled */
}

uint32_t get_flash_offset(uint8_t hw_type)
{
	const hw_t *hwt = get_hw(hw_type);

	if (hwt)
		return hwt->flash_offset;

	return 0; /* disabled */
}

uint32_t get_max_blocksize(uint8_t hw_type)
{
	const hw_t *hwt = get_hw(hw_type);

	if (hwt)
		return hwt->max_blocksize;

	return 0; /* disabled */
}

uint32_t has_hw_flags(uint8_t hw_type, const uint32_t flags)
{
	const hw_t *hwt = get_hw(hw_type);

	if (hwt)
		return hwt->flags & flags;

	return 0; /* disabled by default */
}

const char *get_hw_name(uint8_t hw_type)
{
	const hw_t *hwt = get_hw(hw_type);

	if (hwt)
		return hwt->name;

	return "unkown";
}

int get_num_flashblocks(uint8_t hw_type)
{
	const hw_t *hwt = get_hw(hw_type);

	if (hwt)
		return hwt->num_flashblocks;

	return 0;
}

int check_flash_id_type(uint8_t hw_type, uint8_t flash_id_type)
{
	const hw_t *hwt = get_hw(hw_type);

	if (hwt)
		return (hwt->flash_id_type != flash_id_type);
	else
		return 1; /* no hardware type found => always fail */
}
