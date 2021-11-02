/*
 * pcanfunc.h - flash program for PCAN routers
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
#include <linux/can.h>

int query_modules(int s, struct can_frame *modules);
void init_set_cmd(struct can_frame *frame);
void set_startaddress(int s, uint8_t module_id, uint32_t addr);
void set_blocksize(int s, uint8_t module_id, uint32_t size);
void set_checksum(int s, uint8_t module_id, uint16_t csum);
void erase_sector(int s, uint8_t module_id);
void start_programming(int s, uint8_t module_id);
void verify(int s, uint8_t module_id);
void switch_to_bootloader(int s, uint8_t module_id);
void reset_module(int s, uint8_t module_id);
void end_programming(int s, uint8_t module_id);
uint8_t get_status(int s, uint8_t module_id, struct can_frame *cf);
void write_block(int s, uint8_t module_id, uint32_t offset, uint32_t blksz, uint8_t *buf, uint32_t alternating_xor_flip);
void erase_block(int s, uint8_t module_id, uint32_t startaddr, uint32_t blksz);
