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
#include <string.h>

#include "encodings.h"
#include "loader.h"

static void rom_size(uint8_t index) {
  char rs[20];
  switch(index) {
  case 0x0a:
    strncpy(rs, "8KB (unused)", 20);
    break;
  case 0x0b:
    strncpy(rs, "16KB (unused)", 20);
    break;
  case 0x0c:
    strncpy(rs, "32KB", 20);
    break;
  case 0x0d:
    strncpy(rs, "48KB (unused, buggy)", 20);
    break;
  case 0x0e:
    strncpy(rs, "64KB (rarely used)", 20);
    break;
  case 0x0f:
    strncpy(rs, "128KB", 20);
    break;
  case 0x00:
    strncpy(rs, "256KB", 20);
    break;
  case 0x01:
    strncpy(rs, "512KB (rarely used)", 20);
    break;
  case 0x2:
    strncpy(rs, "1MB (unused, buggy)", 20);
    break;
  }
  printf("ROM Size:\t%s\n", rs);
}

uint8_t* loader_load_rom(const char* path, uint8_t* rom_buffer) {
  size_t blocks = 0;
  uint16_t i;
  uint8_t region;

  FILE* rom_fd = fopen(path, "r");
  if (rom_fd == NULL) {
    printf("Could not open file: %s\n", path);
    return NULL;
  }

  blocks = fread(rom_buffer, sizeof(uint8_t), 512 * 1024, rom_fd);
  if (blocks < 512 * 1024) {
    printf("Could only read %u Bytes\n", (unsigned int)blocks);
    return NULL;
  }

  printf("Loaded rom from %s\n", path);
  /*TODO: Determine right size of loop using sizeof*/
  for(i = 0; i < 3; i++) {
    if(strncmp((const char *)SEGA_STRING, (const char *)&rom_buffer[HEADER_LOC[i]], sizeof(SEGA_STRING)) == 0) {
      printf("Found header @0x%x\n", HEADER_LOC[i]);
      printf("--------------------\n");
      printf("SEGA String:\t%02x %02x %02x %02x %02x %02x %02x %02x (%s)\n",
          rom_buffer[HEADER_LOC[i]],
          rom_buffer[HEADER_LOC[i] + 1],
          rom_buffer[HEADER_LOC[i] + 2],
          rom_buffer[HEADER_LOC[i] + 3],
          rom_buffer[HEADER_LOC[i] + 4],
          rom_buffer[HEADER_LOC[i] + 5],
          rom_buffer[HEADER_LOC[i] + 6],
          rom_buffer[HEADER_LOC[i] + 7],
          &rom_buffer[HEADER_LOC[i]]);
      printf("Reserved:\t%02x %02x\n",
          rom_buffer[HEADER_LOC[i] + 8],
          rom_buffer[HEADER_LOC[i] + 9]);
      printf("Checksum:\t%02x %02x\n",
          rom_buffer[HEADER_LOC[i] + 10],
          rom_buffer[HEADER_LOC[i] + 11]);
      printf("Product Code:\t%02x %02x %02x\n",
          rom_buffer[HEADER_LOC[i] + 12],
          rom_buffer[HEADER_LOC[i] + 13],
          rom_buffer[HEADER_LOC[i] + 14] & 0xf0);
      printf("Version:\t%02x\n",
          rom_buffer[HEADER_LOC[i] + 14] & 0x0f);
      region = (rom_buffer[HEADER_LOC[i] + 15] & 0xf0) >> 4;
      printf("Region code:\t%02x (%s)\n", region, REGION[region]);
      rom_size((rom_buffer[HEADER_LOC[i] + 15] & 0x0f));
      break;
    } else {
      printf("Could not find header @0x%x\n", HEADER_LOC[i]);
    }
  }
  return rom_buffer;
}
