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
#include <stdbool.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "../include/z80.h"
#include "../include/loader.h"
#include "../include/graphics.h"

extern SDL_Window *G_window;
extern SDL_Renderer *G_renderer;

static void show_help(char* app_name) {
    printf("%s -r <rom file>\n", app_name);
}

int main(int argc, char* argv[]) {
    SDL_Event e;
    int c;
    bool quit = false;
    const char* rom_path = "rom/mega_man.gg";
    uint8_t* rom_ptr = NULL;

    while ((c = getopt(argc, argv, "h?")) != -1) {
        switch (c) {
        case 'h':
        case '?':
        default:
            show_help(argv[0]);
        }
    }
    rom_ptr = malloc(512 * sizeof(uint8_t));
    rom_ptr = loader_load_rom(rom_path, rom_ptr);
    if (rom_ptr == NULL) {
        printf("Could not load rom from %s\n", rom_path);
        /*return -1;*/
    }

    /* Setup system state */
    z80_init();
    /* Init graphic subsystem of Game Gear */
    gg_graphics_init();

    while(!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;
            /*if (e.type == SDL_KEYDOWN)
                quit = true;*/
            if (e.type == SDL_MOUSEBUTTONDOWN)
                quit = true;
        }
    }

    /* Main system loop */
    /*while(1) {
        z80_emulate_cycle();
    }*/

    return 0;
}
