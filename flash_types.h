/*
 * flash_types.h - flash type block layouts
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

/* flash block layouts */

const fblock_t flashid12[] = {
	{0x2000, 0x2000, false},
	{0x4000, 0x2000, false},
	{0x6000, 0x2000, false},
	{0x8000, 0x2000, false},
	{0xA000, 0x2000, false},
	{0xC000, 0x2000, false},
	{0xE000, 0x2000, false},
	{0x10000, 0x10000, false},
	{0x20000, 0x10000, false},
	{0x30000, 0x2000, false},
	{0x32000, 0x2000, false},
	{0x34000, 0x2000, false},
	{0x36000, 0x2000, false},
	{0x38000, 0x2000, false},
	{0x3A000, 0x2000, false},
	{0x3C000, 0x2000, false}
};

const fblock_t flashid40[] = {
	{0x0, 0x1000, false},
	{0x1000, 0x1000, true},
	{0x2000, 0x1000, true},
	{0x3000, 0x1000, true},
	{0x4000, 0x1000, true},
	{0x5000, 0x1000, false},
	{0x6000, 0x1000, false},
	{0x7000, 0x1000, false},
	{0x8000, 0x1000, false},
	{0x9000, 0x1000, false},
	{0xA000, 0x1000, false},
	{0xB000, 0x1000, false},
	{0xC000, 0x1000, false},
	{0xD000, 0x1000, false},
	{0xF000, 0x1000, false},
	{0x10000, 0x8000, false},
	{0x18000, 0x8000, false},
	{0x20000, 0x8000, false},
	{0x28000, 0x8000, false},
	{0x30000, 0x8000, false},
	{0x38000, 0x8000, false},
	{0x40000, 0x8000, false},
	{0x48000, 0x8000, false},
	{0x50000, 0x8000, false},
	{0x58000, 0x8000, false},
	{0x60000, 0x8000, false},
	{0x68000, 0x8000, false},
	{0x70000, 0x8000, false},
	{0x78000, 0x8000, false},
	{0x80000, 0x1000, false},
	{0x81000, 0x04A600, false},
	{0x0CB600, 0x334A00, true},
	{0x400000, 0x400000, false}
};

const fblock_t unknownflashid[] = {
};
