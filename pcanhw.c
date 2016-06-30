/*
 * pcanhw.c - flash program for PCAN routers
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
#include <stdlib.h>
#include <string.h>
#include "pcanhw.h"
#include "pcanfunc.h"

/* flash block layouts */

const fblock_t hw4[] = {
	{0x2000, 0x2000},
	{0x4000, 0x2000},
	{0x6000, 0x2000},
	{0x8000, 0x2000},
	{0xA000, 0x2000},
	{0xC000, 0x2000},
	{0xE000, 0x2000},
	{0x10000, 0x10000},
	{0x20000, 0x10000},
	{0x30000, 0x2000},
	{0x32000, 0x2000},
	{0x34000, 0x2000},
	{0x36000, 0x2000},
	{0x38000, 0x2000},
	{0x3A000, 0x2000},
	{0x3C000, 0x2000}
};

const fblock_t hw16[] = {
	{0x2000, 0x2000},
	{0x4000, 0x2000},
	{0x6000, 0x2000},
	{0x8000, 0x2000},
	{0xA000, 0x2000},
	{0xC000, 0x2000},
	{0xE000, 0x2000},
	{0x10000, 0x10000},
	{0x20000, 0x10000},
	{0x30000, 0x2000},
	{0x32000, 0x2000},
	{0x34000, 0x2000},
	{0x36000, 0x2000},
	{0x38000, 0x2000},
	{0x3A000, 0x2000},
	{0x3C000, 0x2000}
};

const fblock_t hw19[] = {
};

const fblock_t hw21[] = {
};

const fblock_t hw25[] = {
	{0x2000, 0x2000},
	{0x4000, 0x2000},
	{0x6000, 0x2000},
	{0x8000, 0x2000},
	{0xA000, 0x2000},
	{0xC000, 0x2000},
	{0xE000, 0x2000},
	{0x10000, 0x10000},
	{0x20000, 0x10000},
	{0x30000, 0x2000},
	{0x32000, 0x2000},
	{0x34000, 0x2000},
	{0x36000, 0x2000},
	{0x38000, 0x2000},
	{0x3A000, 0x2000},
	{0x3C000, 0x2000}
};

const fblock_t hw31[] = {
	{0x2000, 0x2000},
	{0x4000, 0x2000},
	{0x6000, 0x2000},
	{0x8000, 0x2000},
	{0xA000, 0x2000},
	{0xC000, 0x2000},
	{0xE000, 0x2000},
	{0x10000, 0x10000},
	{0x20000, 0x10000},
	{0x30000, 0x2000},
	{0x32000, 0x2000},
	{0x34000, 0x2000},
	{0x36000, 0x2000},
	{0x38000, 0x2000},
	{0x3A000, 0x2000},
	{0x3C000, 0x2000}
};

const fblock_t hw34[] = {
};

const fblock_t hw35[] = {
	{0x2000, 0x2000},
	{0x4000, 0x2000},
	{0x6000, 0x2000},
	{0x8000, 0x2000},
	{0xA000, 0x2000},
	{0xC000, 0x2000},
	{0xE000, 0x2000},
	{0x10000, 0x10000},
	{0x20000, 0x10000},
	{0x30000, 0x2000},
	{0x32000, 0x2000},
	{0x34000, 0x2000},
	{0x36000, 0x2000},
	{0x38000, 0x2000},
	{0x3A000, 0x2000},
	{0x3C000, 0x2000}
};

const fblock_t hw36[] = {
};

const fblock_t hw37[] = {
};

const fblock_t hw40[] = {
};

/* hardware type descriptions */

const hw_t hwt4 = {"PCAN-Micromod", "PCAN_MicroMod",
		   (FDATA_INVERT | SWITCH_TO_BOOTLOADER | RESET_AFTER_FLASH), 0,
		   sizeof(hw4) / sizeof(fblock_t),
		   hw4};

const hw_t hwt16 = {"PCAN-Router", "PCAN-Router",
		   (FDATA_INVERT), 0x03DF00,
		   sizeof(hw16) / sizeof(fblock_t),
		   hw16};

const hw_t hwt19 = {"PCAN-MIO (32-bit)", "PCAN_MIOV4",
		   (FDATA_INVERT | SWITCH_TO_BOOTLOADER | RESET_AFTER_FLASH), 0xFF9000,
		   sizeof(hw19) / sizeof(fblock_t),
		   hw19};

const hw_t hwt21 = {"MU-Thermocouple1 CAN", "PCAN-MU",
		   (FDATA_INVERT | SWITCH_TO_BOOTLOADER | RESET_AFTER_FLASH), 0x002000,
		   sizeof(hw21) / sizeof(fblock_t),
		   hw21};

const hw_t hwt25 = {"PCAN-Router Pro", "PCAN-Router_Pro",
		   (FDATA_INVERT | SWITCH_TO_BOOTLOADER | RESET_AFTER_FLASH), 0x03DF00,
		   sizeof(hw25) / sizeof(fblock_t),
		   hw25};

const hw_t hwt31 = {"PCAN-RS-232", "PCAN-RS-232",
		   (FDATA_INVERT), 0x03DF00,
		   sizeof(hw31) / sizeof(fblock_t),
		   hw31};

const hw_t hwt34 = {"PCAN-USB Pro FD", "PCAN-USB_PRO_FD",
		   (FDATA_INVERT), 0x000100,
		   sizeof(hw34) / sizeof(fblock_t),
		   hw34};

const hw_t hwt35 = {"PCAN-Router DR", "PCAN-Router-DR",
		   (FDATA_INVERT), 0x03DF00,
		   sizeof(hw35) / sizeof(fblock_t),
		   hw35};

const hw_t hwt36 = {"PCAN-USB FD", "PCAN-USB_FD",
		   (FDATA_INVERT), 0x000100,
		   sizeof(hw36) / sizeof(fblock_t),
		   hw36};

const hw_t hwt37 = {"PCAN-GPS", "PCAN-GPS",
		   (FDATA_INVERT), 0x000100,
		   sizeof(hw37) / sizeof(fblock_t),
		   hw37};

const hw_t hwt40 = {"PCAN-Router FD", "PCAN-Router_FD",
		   (FDATA_INVERT), 0,
		   sizeof(hw40) / sizeof(fblock_t),
		   hw40};

/* index table for fast hw_type referencing */

const hw_t *hwtab[] =
{
	NULL,	NULL,	NULL,	NULL,	&hwt4,	NULL,	NULL,	NULL,
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,
	&hwt16,	NULL,	NULL,	&hwt19,	NULL,	&hwt21,	NULL,	NULL,
	NULL,	&hwt25,	NULL,	NULL,	NULL,	NULL,	NULL,	&hwt31,
	NULL,	NULL,	&hwt34,	&hwt35,	&hwt36,	&hwt37,	NULL,	NULL,
	&hwt40
};

const hw_t *get_hw(uint8_t hw_type)
{
	if (hw_type > 40)
		return NULL; /* unknown */

	return hwtab[hw_type];
}

uint32_t crc_startpos(uint8_t hw_type)
{
	const hw_t *hwt = get_hw(hw_type);

	if (hwt)
		return hwt->crc_startpos;

	return 0; /* disabled */
}

uint32_t hw_flags(uint8_t hw_type, const uint32_t flags)
{
	const hw_t *hwt = get_hw(hw_type);

	if (hwt)
		return hwt->flags & flags;

	return 0; /* disabled by default */
}

const char *hw_name(uint8_t hw_type)
{
	const hw_t *hwt = get_hw(hw_type);

	if (hwt)
		return hwt->name;

	return "unkown";
}

int num_flashblocks(uint8_t hw_type)
{
	const hw_t *hwt = get_hw(hw_type);

	if (hwt)
		return hwt->num_flashblocks;

	return 0;
}

const char *flash_name(uint8_t flash_type)
{
	switch (flash_type) {
	case 4:
		return "MB90F497G";
	case 12:
		return "LPC_IAP";
	case 14:
		return "M32C87_1MB";
	case 16:
		return "LPC23xx_256k";
	case 20:
		return "USB_PRO_FD_Flash";
	case 21:
		return "USB_FD_Flash";
	case 22:
		return "LPC407X_8X";
	case 40:
		return "FLASH_ROUTER_FD";
	default:
		return "unkown";
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
			hw_type, hw_name(hw_type));
		exit(1);
	}

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
