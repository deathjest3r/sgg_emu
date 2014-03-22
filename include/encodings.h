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

#ifndef __ENCODING_H__
#define __ENCODING_H__

#define A 0x7 /*111*/
#define B 0x0 /*000*/
#define C 0x1 /*001*/
#define D 0x2 /*010*/
#define E 0x3 /*011*/
#define H 0x4 /*100*/
#define L 0x5 /*101*/

#define reg_B   0
#define reg_C   1
#define reg_D   2
#define reg_E   3
#define reg_H   4
#define reg_L   5
#define reg_B_  6
#define reg_C_  7
#define reg_D_  8
#define reg_E_  9
#define reg_H_  10
#define reg_L_  11

/* Elements in the flag register */
#define CARRY_FLAG              0x01 /*C*/
#define ADDSUB_FLAG             0x02 /*N*/
#define PARITYOVERFLOW_FLAG     0x03 /*P/V*/
#define HALFCARRY_FLAG          0x10 /*H*/
#define ZERO_FLAG               0x40 /*Z*/
#define SIGN_FLAG               0x80 /*S*/

#endif /*__ENCODING_H__*/
