
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
