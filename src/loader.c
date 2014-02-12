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

#include "../include/loader.h"


uint8_t* loader_load_rom(const char* path) {
    uint8_t rom_buffer[2048];
    size_t blocks = 0;
    
    FILE* rom_fd = fopen(path, "r");
    if (rom_fd == NULL) {
        printf("Could not open file: %s\n", path);
        return NULL;
    }

    blocks = fread(&rom_buffer, sizeof(uint8_t), 2048, rom_fd);
    if (blocks < 2048) {
        printf("Could only read %u Bytes\n", (unsigned int)blocks);
        return NULL;
    }

    printf("%s\n", path);
    return NULL;
}
