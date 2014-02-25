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

#include <stdio.h>
#include <stdint.h>

#include "../include/z80.h"
#include "../include/loader.h"

int main(int argc, char** argv) {
    const char* rom_path = "rom/mega_man.gg";
    uint8_t* rom_ptr = loader_load_rom(rom_path);

    (void)argc; (void)argv;

    if (rom_ptr == NULL) {
        printf("Could not load rom from %s\n", rom_path);
        return -1;
    }

    /* Setup system state */
    z80_init();
    z80_graphics_init();

    /* Main system loop */
    /*while(1) {
        z80_emulate_cycle();
    }*/

    return 0;
}
