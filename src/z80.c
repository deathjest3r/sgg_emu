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

#include "../include/encodings.h"
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
    uint8_t ram[RAM_SZ];  /* 8KB RAM */
    uint8_t vram[VRAM_SZ]; /* 16KB VRAM */
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

int z80_gp_valid(uint8_t reg) {
    if ((reg == 0x6) || (reg > 0x7)) {
        return 0;
    }
    return 1;
}

uint8_t z80_get_t_reg(uint8_t operand) {
    uint8_t t_reg = (operand & 0x38) >> 3;
    if (!z80_gp_valid(t_reg)) {
        printf("Unkown target register %u\n", t_reg);
        return 0;
    }
    return t_reg;
}

uint8_t z80_get_s_reg(uint8_t operand) {
    uint8_t s_reg = (operand & 0x7);
    if (!z80_gp_valid(s_reg)) {
        printf("Unkown source register %u\n", s_reg);
        return 0;
    }
    return s_reg;
}

int z80_ram_valid(uint16_t value) {
    if (value >= RAM_SZ) {
        return 0;
    }
    return 1;
}

uint8_t z80_fetch_byte(void) {
    /* Check if PC points to valid position */
    if (!z80_ram_valid(z80_state.pc)) {
        printf("Unkown PC position %u\n", z80_state.pc);
        return 0;
    }
    /* Increment PC after returning instruction */
    return z80_state.pc++;
}

/***
 * The Z80 CPU can execute 158 different instruction types including all 78 of
 * the 8080A CPU
 */
void z80_decode_inst() {
    uint8_t operand_1, operand_2, operand_3;
    uint16_t s_reg, t_reg;

    operand_1 = z80_fetch_byte();

    /* LD r, r' */
    if ((operand_1 & 0xC0) == 0x40) {
        /* Get source and destination registers */
        s_reg = z80_get_s_reg(operand_1);
        t_reg = z80_get_t_reg(operand_1);

        /* Store value of source register in target register*/
        z80_state.gp[t_reg] = z80_state.gp[s_reg];

    /* LD r, n */
    } else if ((operand_1 & 0xC7) == 0x06) {
        /* Load second operand from memory */
        operand_2 = z80_fetch_byte();

        /* Determine target register */
        t_reg = z80_get_t_reg(operand_1);

        /* Store operand_2 in target register */
        z80_state.gp[t_reg] = operand_2;

    /* LD r, (HL) */
    } else if ((operand_1 & 0xC7) == 0x46) {

        t_reg = z80_get_t_reg(operand_1);

        s_reg = ((z80_state.gp[4] << 8) & z80_state.gp[5]);

        if (!z80_ram_valid(s_reg)) {
            printf("Unkown source RAM address %u for LD instruction\n",
                    s_reg);
        }
        z80_state.gp[t_reg] = z80_state.ram[s_reg];

    /* LD r, (IX+d) */
    } else if (operand_1 == 0xDD) {
        operand_2 = z80_fetch_byte();
        t_reg = z80_get_t_reg(operand_2);
        operand_3 = z80_fetch_byte();

        z80_state.gp[t_reg] = (z80_state.ix + operand_3);

    /* LD r, (IY+d) */
    } else if (operand_1 == 0xFD) {
        operand_2 = z80_fetch_byte();
        t_reg = z80_get_t_reg(operand_2);
        operand_3 = z80_fetch_byte();

        z80_state.gp[t_reg] = (z80_state.iy + operand_3);

    /* LD (HL), r */
    } else if ((operand_1 & 0xF8) == 0x70) {
    } else if (operand_1 == 0xDD) {
        operand_2 = z80_fetch_byte();
        /* LD (IX+d), r */
        if ((operand_2 & 0xF8) == 0x70) {
        /* LD (IX+d), n */
        } else if (operand_2 == 0x36) {
        /* LD IX, nn */
        } else if (operand_2 == 0x21) {
        /* LD IX, (nn) */
        } else if (operand_2 == 0x2A) {
        /* LD (nn), IX */
        } else if (operand_2 == 0x22) {
        /* LD SP, IX */
        } else if (operand_2 == 0xF9) {
        /* PUSH IX */
        } else if (operand_2 == 0xE5) {
        /* POP IX */
        } else if (operand_2 == 0xE1) {
        }
    } else if(operand_1 == 0xFD) {
        operand_2 = z80_fetch_byte();
        /* LD (IY+d), r*/
        if((operand_2 & 0xF8) == 0x70) {
        /* LD (IY+d), n */
        } else if (operand_2 == 0x36) {
        /* LD IY, nn */
        } else if (operand_2 == 0x21) {
        /* LD IY, (nn) */
        } else if (operand_2 == 0x2A) {
        /* LD (nn), IY */
        } else if (operand_2 == 0x22) {
        /* LD SP, IY */
        } else if (operand_2 == 0xF9) {
        /* PUSH IY */
        } else if (operand_2 == 0xE5) {
        /* POP IY */
        } else if (operand_2 == 0xE1) {
        }
    /* LD (HL), n */
    } else if (operand_1 == 0x36) {
    /* LD A, (BC) */
    } else if (operand_1 == 0x0A) {
    /* LD A, (DE) */
    } else if (operand_1 == 0x1A) {
    /* LD A, (nn) */
    } else if (operand_1 == 0x3A) {
    /* LD (BC), A */
    } else if (operand_1 == 0x02) {
    /* LD (DE), A */
    } else if (operand_1 == 0x12) {
    /* LD (nn), A */
    } else if (operand_1 == 0x32) {
    } else if (operand_1 == 0xED) {
        operand_2 = z80_fetch_byte();
        /* LD A, I */
        if (operand_2 == 0x57) {
        /* LD A, R */
        } else if (operand_2 == 0x5F) {
        /* LD I, A */
        } else if (operand_2 == 0x47) {
        /* LD R, A */
        } else if (operand_2 == 0x4F) {
        /* LD dd, (nn) */
        } else if ((operand_2 & 0xCF) == 0x4B) {
        /* LD (nn), dd */
        } else if ((operand_2 & 0xCF) == 0x43) {
        }
    /* LD dd, nn */
    } else if ((operand_1 & 0xCF) == 0x1) {
    /* LD HL, (nn) */
    } else if (operand_1 == 0x2A) {
    /* LD (nn), HL */
    } else if (operand_1 == 0x22) {
    /* LD SP, HL */
    } else if (operand_1 == 0xF9) {
    /* PUSH qq */
    } else if ((operand_1 & 0xCF) == 0xC5) {
    /* POP qq */
    } else if ((operand_1 & 0xCF) == 0xC1) {
    /* Block Transfer and Search */
    
    /* EX DE, HL */
    
    /* EX AF, AF' */
    
    /* EXX */
    
    /* EX (SP), HL */
    
    /* EX (SP), IX */
    
    /* EX (SP), IY */
    
    /* LDI */
    
    /* LDIR */
    
    /* LDD */
    
    /* LDDR */
    
    /* CPI */
    
    /* CPIR */
    
    /* CPD */
    
    /* CPDR */

    /* Arithmetic and Logical */
    /* ADD A, r */
    /* ADD A, n */
    /* ADD A, (HL) */
    /* ADD A, (IX+d) */
    /* ADD A, (IY+d) */
    /* ADC A, s */
    /* SUB s */
    /* SBC A, s */
    /* AND s */
    /* OR s */
    /* XOR s */
    /* CP s*/
    /* INC r */
    /* INC (HL) */
    /* INC (IX+d) */
    /* INC (IY+d) */
    /* DEC m */

    /* DAA */
    /* CPL */
    /* NEG */
    /* CCF */
    /* SCF */
    /* NOP */
    /* HALT */
    /* DI */
    /* EI */
    /* IM 0 */
    /* IM 1 */
    /* IM 2 */

    /* ADD HL, ss */
    /* ADC HL, ss */
    /* SBC HL, ss */
    /* ADD IX, pp */
    /* ADD IY, rr */
    /* INC ss */
    /* INC IX */
    /* INC IY */
    /* DEC ss */
    /* DEC IX */
    /* DEC IY */

    /* Rotate and Shift */
    /* RLCA */
    /* RLA */
    /* RLCA */
    /* RRA */
    /* RLC r */
    /* RLC (HL) */
    /* RLC (IX+d) */
    /* RLC (IY+d) */
    /* RL m */
    /* RRC m */
    /* RR m */
    /* SLA m */
    /* SRA m */
    /* SRL m */
    /* RLD */
    /* RRD */

    /* Bit Manipulation (Set, Reset, Test) */
    /* BIT b, r */
    /* BIT b, (HL) */
    /* BIT b, (IX+d) */
    /* BIT b, (IY+d) */
    /* SET b, r */
    /* SET b, (HL) */
    /* SET b, (IX+d) */
    /* SET b, (IY+d) */
    /* RES b, m */

    /* Jump, Call, and Return */
    /* JP nn */
    /* JP cc, nn */
    /* JR e */
    /* JR C, e */
    /* JR NC, e */
    /* JR Z, e */
    /* JR NZ, e */
    /* JP (HL) */
    /* JP (IX) */
    /* JP (IY) */
    /* DJNZ, e */
    /* CALL nn */
    /* CALL cc, nn */
    /* RET */
    /* RET cc */
    /* RETI */
    /* RETN */
    /*RST p */

    /* Input/Output */
    /* IN A, (n) */
    /* IN r (C)*/
    /* INI */
    /* INIR */
    /* IND */
    /* INDR */
    /* OUT (n), A */
    /* OUT (C), r */
    /* OUTI */
    /* OTIR */
    /* OUTD */
    /* OTDR */
    }

}

