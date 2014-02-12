
#include <stdint.h>

#include "../include/z80.h"

/***
* Struct which holds the Z80's processor state
*/
struct Z80_State {
    uint8_t gp[12]; /* General Purpose Registers */
    uint8_t acc;    /* Accumulator */
    uint8_t flags;  /* Flags register */
    uint8_t r;      /* Memory Refresh */
    uint8_t i;      /* Interrupt Vector */
    uint16_t ix;    /* Index Register */
    uint16_t iy;    /* Index Register */
    uint16_t sp;    /* Stack Pointer */
    uint16_t pc;    /* Program Counter */
    uint8_t ram[8192];  /* 8KB RAM */
    uint8_t vram[16384]; /* 16KB VRAM */
} z80_state;

void z80_emulate_cycle(void) {
    /* Fetch OpCode */
    /* Decode OpCode */
    /* Execute OpCode */
    /* Update Timers */
    return;
}

void z80_graphics_init(void) {
    return;
}
     

void z80_init(void) {
    /* Set PC to first address in RAM */
    z80_state.pc = z80_state.ram[0];
    return;
}


/***
 * The Z80 CPU can execute 158 different instruction types including all 78 of
 * the 8080A CPU
 */
void z80_decode_inst() {

    switch(z80_state.pc) {
        /* Load and Exchanged */
        case 0xDD:
        break;
        
        /* Block Transfer and Search */
        /* Arithmetic and Logical */
        /* Rotate and Shift */
        /* Bit Manipulation (Set, Reset, Test) */
        /* Jump, Call, and Return */
        /* Input/Output */
        /* Basic CPU Control */
    }
    
}

