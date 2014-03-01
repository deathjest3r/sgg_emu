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

#include <SDL2/SDL.h>

#include "../include/graphics.h"

SDL_Window *G_window = NULL;
SDL_Renderer *G_renderer = NULL;

void gg_graphics_init() {

     if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
        printf("%s\n", SDL_GetError());
        return;
    }

    G_window = SDL_CreateWindow("SGGEmu", 100, 100, GG_WIDTH, GG_HEIGHT, SDL_WINDOW_SHOWN);
    if (G_window == NULL) {
        printf("%s\n", SDL_GetError());
        return;
    }

    G_renderer = SDL_CreateRenderer(G_window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (G_renderer == NULL) {
        printf("%s\n", SDL_GetError());
        return;
    }

    SDL_Delay(2000);
}

void gg_graphics_destroy() {
    SDL_DestroyRenderer(G_renderer);
    SDL_DestroyWindow(G_window);
    SDL_Quit();
}
