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
#include "pcanhw.h"
#include "pcanfunc.h"

fblock_t hw16[] = {
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

char *hw_name(uint8_t hw_type)
{
	switch (hw_type) {
	case 4:
		return "PCAN-MicroMod";
	case 16:
		return "PCAN-Router";
	case 19:
		return "PCAN-MIO (32-bit)";
	case 21:
		return "MU-Thermocouple1 CAN";
	case 25:
		return "PCAN-Router Pro";
	case 31:
		return "PCAN-RS-232";
	case 35:
		return "PCAN-Router DR";
	default:
		return "unkown";
	}
}

char *flash_name(uint8_t flash_type)
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
	default:
		return "unkown";
	}
}

int num_flashblocks(uint8_t hw_type)
{
	switch (hw_type) {
	case 16:
		/* PCAN-Router */
		return sizeof(hw16) / sizeof(hw16[0]);
		break;
	default:
		return 0;
	}
}

void erase_flashblocks(int s, FILE *infile, uint8_t module_id, uint8_t hw_type, int index)
{
	uint32_t startaddr, blksz;
	uint8_t data;
	int i;

	switch (hw_type) {
	case 16:
		/* PCAN-Router */
		startaddr = hw16[index].start;
		blksz = hw16[index].len;
		break;
	default:
		fprintf(stderr, "bad flashblocks entry found for hardware type %d (%s)!\n",
			hw_type, hw_name(hw_type));
		exit(1);
	}

	/* check block in bin-file */
	fseek(infile, startaddr, SEEK_SET);

	for (i = 0; i < blksz; i++) {
		if (fread(&data, 1, 1, infile) != 1) {
			/* file ended but was empty so far -> no action */
			return;
		}
		if (data != 0xFFU)
			break;
	}

	/* empty block (all bytes are 0xFFU) -> no action */
	if (i == blksz)
		return;

	erase_block(s, module_id, startaddr, blksz);
}
