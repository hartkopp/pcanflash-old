/*
 * hardware_types.h - hardware descriptions
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

/* hardware type descriptions */

const hw_t hwt4 = {"PCAN-Micromod", "PCAN_MicroMod",
		   (FDATA_INVERT | SWITCH_TO_BOOTLOADER | END_PROGRAMMING | RESET_AFTER_FLASH),
		   0, /* CRC start */
		   0xFF0000, /* flash offset */
		   64, /* max blocksize */
		   4, /* Flash ID type */
		   sizeof(flashid4) / sizeof(fblock_t),
		   flashid4};

const hw_t hwt16 = {"PCAN-Router", "PCAN-Router",
		   (FDATA_INVERT | DATA_MODE8),
		   0x03DF00, /* CRC start */
		   0, /* flash offset */
		   512, /* max blocksize */
		   12, /* Flash ID type */
		   sizeof(flashid12) / sizeof(fblock_t),
		   flashid12};

const hw_t hwt19 = {"PCAN-MIO (32-bit)", "PCAN_MIOV4",
		   (FDATA_INVERT | SWITCH_TO_BOOTLOADER | RESET_AFTER_FLASH),
		   0xFF9000, /* CRC start */
		   0, /* flash offset */
		   256, /* max blocksize */
		   UNKNOWN_FLASH_ID, /* Flash ID type */
		   sizeof(unknownflashid) / sizeof(fblock_t),
		   unknownflashid};

const hw_t hwt21 = {"MU-Thermocouple1 CAN", "PCAN-MU",
		   (FDATA_INVERT | SWITCH_TO_BOOTLOADER | RESET_AFTER_FLASH),
		   0x002000, /* CRC start */
		   0, /* flash offset */
		   512, /* max blocksize */
		   UNKNOWN_FLASH_ID, /* Flash ID type */
		   sizeof(unknownflashid) / sizeof(fblock_t),
		   unknownflashid};

const hw_t hwt25 = {"PCAN-Router Pro", "PCAN-Router_Pro",
		   (FDATA_INVERT | DATA_MODE8 | SWITCH_TO_BOOTLOADER | RESET_AFTER_FLASH),
		   0x03DF00, /* CRC start */
		   0, /* flash offset */
		   512, /* max blocksize */
		   12, /* Flash ID type */
		   sizeof(flashid12) / sizeof(fblock_t),
		   flashid12};

const hw_t hwt31 = {"PCAN-RS-232", "PCAN-RS-232",
		   (FDATA_INVERT | DATA_MODE8),
		   0x03DF00, /* CRC start */
		   0, /* flash offset */
		   512, /* max blocksize */
		   12, /* Flash ID type */
		   sizeof(flashid12) / sizeof(fblock_t),
		   flashid12};

const hw_t hwt35 = {"PCAN-Router DR", "PCAN-Router-DR",
		   (FDATA_INVERT | DATA_MODE8),
		   0x03DF00, /* CRC start */
		   0, /* flash offset */
		   512, /* max blocksize */
		   12, /* Flash ID type */
		   sizeof(flashid12) / sizeof(fblock_t),
		   flashid12};

const hw_t hwt37 = {"PCAN-GPS", "PCAN-GPS",
		   (FDATA_INVERT | DATA_MODE8),
		   0x000100, /* CRC start */
		   0, /* flash offset */
		   512, /* max blocksize */
		   UNKNOWN_FLASH_ID, /* Flash ID type */
		   sizeof(unknownflashid) / sizeof(fblock_t),
		   unknownflashid};

const hw_t hwt40 = {"PCAN-Router FD", "PCAN-Router_FD",
		   (FDATA_INVERT | DATA_MODE8 | END_PROGRAMMING),
		   0, /* CRC start */
		   0, /* flash offset */
		   512, /* max blocksize */
		   40, /* Flash ID type */
		   sizeof(flashid40) / sizeof(fblock_t),
		   flashid40};

const hw_t hwt42 = {"PCAN-Router Pro FD", "PCAN-Router_Pro_FD",
		   (FDATA_INVERT | DATA_MODE8 | END_PROGRAMMING),
		   0, /* CRC start */
		   0, /* flash offset */
		   512, /* max blocksize */
		   42, /* Flash ID type */
		   sizeof(flashid42) / sizeof(fblock_t),
		   flashid42};

const hw_t hwt43 = {"PCAN-MiniDiag FD", "PCAN-MiniDiag-FD",
		   (FDATA_INVERT | DATA_MODE8),
		   0, /* CRC start */
		   0, /* flash offset */
		   256, /* max blocksize */
		   UNKNOWN_FLASH_ID, /* Flash ID type */
		   sizeof(unknownflashid) / sizeof(fblock_t),
		   unknownflashid};

const hw_t hwt44 = {"PCAN-MicroMod FD", "PCAN-MicroMod-FD",
		   (FDATA_INVERT),
		   0, /* CRC start */
		   0, /* flash offset */
		   256, /* max blocksize */
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

/* hardware type flash names */

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
