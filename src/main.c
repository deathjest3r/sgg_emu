#include <stdio.h>
#include <stdint.h>

#include "../include/z80.h"
#include "../include/loader.h"

int main(int argc, char** argv) {
    const char* rom_path = "rom/mega_man.gg";

    uint8_t* rom_ptr = loader_load_rom(rom_path);
    if (rom_ptr == NULL) {
        printf("Could not load rom from %s\n", rom_path);
        return -1;
    }
    
    // Setup system state
    z80_init();
    z80_graphics_init();

    // Main system loop
    //while(1) {
    //    z80_emulate_cycle();
    //}

    return 0;
}
