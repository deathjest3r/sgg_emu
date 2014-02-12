/*
 * This file is part of the SGGEmu project.
 *
 * Copyright (C) 2014 Julian Vetter <julian@sec.t-labs.tu-berlin.de>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef __Z_80_H__
#define __Z_80_H__

#define RAM_SZ  8192
#define VRAM_SZ 16384

void z80_init(void);
void z80_graphics_init(void); 
void z80_emulate_cycle(void);

int z80_gp_valid(uint8_t);
int z80_ram_valid(uint8_t);

uint8_t z80_get_t_reg(uint8_t);
uint8_t z80_get_s_reg(uint8_t);
uint8_t z80_fetch_instruction(void);


#endif /*__Z_80_H__*/
