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

const fblock_t flashid12[] = {
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

const fblock_t flashid40[] = {
	{0x0, 0x1000},
	{0x1000, 0x1000},
	{0x2000, 0x1000},
	{0x3000, 0x1000},
	{0x4000, 0x1000},
	{0x5000, 0x1000},
	{0x6000, 0x1000},
	{0x7000, 0x1000},
	{0x8000, 0x1000},
	{0x9000, 0x1000},
	{0xA000, 0x1000},
	{0xB000, 0x1000},
	{0xC000, 0x1000},
	{0xD000, 0x1000},
	{0xF000, 0x1000},
	{0x10000, 0x8000},
	{0x18000, 0x8000},
	{0x20000, 0x8000},
	{0x28000, 0x8000},
	{0x30000, 0x8000},
	{0x38000, 0x8000},
	{0x40000, 0x8000},
	{0x48000, 0x8000},
	{0x50000, 0x8000},
	{0x58000, 0x8000},
	{0x60000, 0x8000},
	{0x68000, 0x8000},
	{0x70000, 0x8000},
	{0x78000, 0x8000},
	{0x80000, 0x1000},
	{0x81000, 0x04A600},
	{0x0CB600, 0x334A00},
	{0x400000, 0x400000}
};

const fblock_t unknownflashid[] = {
};

/* hardware type descriptions */

const hw_t hwt4 = {"PCAN-Micromod", "PCAN_MicroMod",
		   (FDATA_INVERT | SWITCH_TO_BOOTLOADER | RESET_AFTER_FLASH),
		   0, /* CRC start */
		   0, /* flash offset */
		   UNKNOWN_FLASH_ID, /* Flash ID type */
		   sizeof(unknownflashid) / sizeof(fblock_t),
		   unknownflashid};

const hw_t hwt16 = {"PCAN-Router", "PCAN-Router",
		   (FDATA_INVERT),
		   0x03DF00, /* CRC start */
		   0, /* flash offset */
		   12, /* Flash ID type */
		   sizeof(flashid12) / sizeof(fblock_t),
		   flashid12};

const hw_t hwt19 = {"PCAN-MIO (32-bit)", "PCAN_MIOV4",
		   (FDATA_INVERT | SWITCH_TO_BOOTLOADER | RESET_AFTER_FLASH),
		   0xFF9000, /* CRC start */
		   0, /* flash offset */
		   UNKNOWN_FLASH_ID, /* Flash ID type */
		   sizeof(unknownflashid) / sizeof(fblock_t),
		   unknownflashid};

const hw_t hwt21 = {"MU-Thermocouple1 CAN", "PCAN-MU",
		   (FDATA_INVERT | SWITCH_TO_BOOTLOADER | RESET_AFTER_FLASH),
		   0x002000, /* CRC start */
		   0, /* flash offset */
		   UNKNOWN_FLASH_ID, /* Flash ID type */
		   sizeof(unknownflashid) / sizeof(fblock_t),
		   unknownflashid};

const hw_t hwt25 = {"PCAN-Router Pro", "PCAN-Router_Pro",
		   (FDATA_INVERT | SWITCH_TO_BOOTLOADER | RESET_AFTER_FLASH),
		   0x03DF00, /* CRC start */
		   0, /* flash offset */
		   12, /* Flash ID type */
		   sizeof(flashid12) / sizeof(fblock_t),
		   flashid12};

const hw_t hwt31 = {"PCAN-RS-232", "PCAN-RS-232",
		   (FDATA_INVERT),
		   0x03DF00, /* CRC start */
		   0, /* flash offset */
		   12, /* Flash ID type */
		   sizeof(flashid12) / sizeof(fblock_t),
		   flashid12};

const hw_t hwt35 = {"PCAN-Router DR", "PCAN-Router-DR",
		   (FDATA_INVERT),
		   0x03DF00, /* CRC start */
		   0, /* flash offset */
		   12, /* Flash ID type */
		   sizeof(flashid12) / sizeof(fblock_t),
		   flashid12};

const hw_t hwt37 = {"PCAN-GPS", "PCAN-GPS",
		   (FDATA_INVERT),
		   0x000100, /* CRC start */
		   0, /* flash offset */
		   UNKNOWN_FLASH_ID, /* Flash ID type */
		   sizeof(unknownflashid) / sizeof(fblock_t),
		   unknownflashid};

const hw_t hwt40 = {"PCAN-Router FD", "PCAN-Router_FD",
		   (FDATA_INVERT | END_PROGRAMMING),
		   0, /* CRC start */
		   0, /* flash offset */
		   40, /* Flash ID type */
		   sizeof(flashid40) / sizeof(fblock_t),
		   flashid40};

const hw_t hwt42 = {"PCAN-Router Pro FD", "PCAN-Router_Pro_FD",
		   (FDATA_INVERT),
		   0, /* CRC start */
		   0, /* flash offset */
		   UNKNOWN_FLASH_ID, /* Flash ID type */
		   sizeof(unknownflashid) / sizeof(fblock_t),
		   unknownflashid};

const hw_t hwt43 = {"PCAN-MiniDiag FD", "PCAN-MiniDiag-FD",
		   (FDATA_INVERT),
		   0, /* CRC start */
		   0, /* flash offset */
		   UNKNOWN_FLASH_ID, /* Flash ID type */
		   sizeof(unknownflashid) / sizeof(fblock_t),
		   unknownflashid};

const hw_t hwt44 = {"PCAN-MicroMod FD", "PCAN-MicroMod-FD",
		   (FDATA_INVERT),
		   0, /* CRC start */
		   0, /* flash offset */
		   UNKNOWN_FLASH_ID, /* Flash ID type */
		   sizeof(unknownflashid) / sizeof(fblock_t),
		   unknownflashid};

/* index table for fast hw_type referencing */

const hw_t *hwtab[] =
{
	NULL,	NULL,	NULL,	NULL,	&hwt4,	NULL,	NULL,	NULL,
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,
	&hwt16,	NULL,	NULL,	&hwt19,	NULL,	&hwt21,	NULL,	NULL,
	NULL,	&hwt25,	NULL,	NULL,	NULL,	NULL,	NULL,	&hwt31,
	NULL,	NULL,	NULL,	&hwt35,	NULL,	&hwt37,	NULL,	NULL,
	&hwt40,	NULL,	&hwt42,	&hwt43,	&hwt44
};

const hw_t *get_hw(uint8_t hw_type)
{
	if (hw_type > 44)
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

const char *get_flash_name(uint8_t flash_type)
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
	case 42:
		return "FLASH_ROUTER_PRO_FD";
	case 43:
		return "LPC546xx_256k";
	case 44:
		return "LPC546xx_512k";
	default:
		return "unkown";
	}
}

int check_flash_id_type(uint8_t hw_type, uint8_t flash_id_type)
{
	const hw_t *hwt = get_hw(hw_type);

	return (hwt->flash_id_type != flash_id_type);
}
