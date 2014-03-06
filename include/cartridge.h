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

#ifndef __CARTRIDGE_H__
#define __CARTRIDGE_H__

/*0x7ff0 - TMR SEGA*/
TMR_SEGA = [0x54, 0x4D, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41]

/*0x7ff8 - Reserved Space*/

/*0x7ffa - Checksum*/

/*0x7ffc - Product code*/
/*0x7ffe - Version*/
/*0x7fff - Region code*/

/*0x7fff - ROM size*/

#endif /*__CARTRIDGE_H__*/
