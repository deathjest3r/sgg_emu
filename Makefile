#
#  This file is part of the SGGEmu project.
#  
#  Copyright (C) 2014 Julian Vetter <julian@sec.t-labs.tu-berlin.de>
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or 
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
#
CC=gcc
LD=gcc
CFLAGS=-std=gnu99 -g -O0 -Wall -DDEBUG -Wextra -I/opt/local/include -Iinclude
LDFLAGS =
LDLIBS = -L/opt/local/lib -lSDL2
HDR := $(wildcard include/*)
SRC := $(wildcard src/*.c)
OBJ := $(patsubst %.c,%.o,$(SRC))
PROG := sgg_emu

all: $(PROG)

$(PROG): $(OBJ)
	$(LD) $(LDFLAGS) $(LDLIBS) $^ -o $@

$(OBJ): %.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(PROG)
