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
