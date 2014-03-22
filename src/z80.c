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
    uint8_t acc_;   /* A' */
    uint8_t flags_; /* F' */
    uint8_t r;      /* Memory Refresh */
    uint8_t i;      /* Interrupt Vector */
    uint16_t ix;    /* Index Register */
    uint16_t iy;    /* Index Register */
    uint16_t sp;    /* Stack Pointer */
    uint16_t pc;    /* Program Counter */
    uint8_t ram[RAM_SZ];  /* 8KB RAM */
    uint8_t vram[VRAM_SZ]; /* 16KB VRAM */
    uint8_t iff1;   /*Interrupt Enable/Disable Flip-Flop I*/
    uint8_t iff2;   /*Interrupt Enable/Disable Flip-Flop II*/
} z80_state;

void z80_emulate_cycle(void) {
    /* Fetch OpCode */
    /* Decode OpCode */
    /* Execute OpCode */
    /* Update Timers */
    return;
}

void z80_init(void) {
    /* Set PC to first address in RAM */
    z80_state.pc = z80_state.ram[0];
    return;
}

int z80_flag_set(uint8_t mask, uint8_t flag) {
    if ((mask & flag) == flag)
        return 1;
    return 0;
}

void z80_update_flags(uint8_t value, uint8_t mask) {
    if(z80_flag_set(mask, SIGN_FLAG)) {
        /* S is set if result is negative; reset otherwise*/
        if((value & 0x80) == 0x80)
            z80_state.flags |= SIGN_FLAG;
        else
            z80_state.flags &= ~SIGN_FLAG;
    }

    if(z80_flag_set(mask, ZERO_FLAG)) {
        /* Z is set if result is zero; reset otherwise */
        if (value == 0)
            z80_state.flags |= ZERO_FLAG;
        else
            z80_state.flags &= ~ZERO_FLAG;
    }

    if(z80_flag_set(mask, HALFCARRY_FLAG)) {
        /* H is set if carry from bit 3; reset otherwise */
        if((value & 0x10) == 0x10)
            z80_state.flags |= HALFCARRY_FLAG;
        else
            z80_state.flags &= ~HALFCARRY_FLAG;
    }

    if(z80_flag_set(mask, ADDSUB_FLAG)) {
        /* N is reset */
        z80_state.flags &= ~ADDSUB_FLAG;
    }

    if(z80_flag_set(mask, PARITYOVERFLOW_FLAG)) {
        /* Store state of iff2 in parity flag*/
        if(z80_state.iff2)
            z80_state.flags |= PARITYOVERFLOW_FLAG;
        else
            z80_state.flags &= ~PARITYOVERFLOW_FLAG;
    }

    /*TODO: Fix this!*/
    if(z80_flag_set(mask, CARRY_FLAG)) {
        /* C is set if carry from bit 7; reset otherwise */
        if((value & 0x100) == 0x100)
            z80_state.flags |= CARRY_FLAG;
        else
            z80_state.flags &= ~CARRY_FLAG;
    }
}

int z80_gp_valid(uint8_t reg) {
    if ((reg == 0x6) || (reg > 0x7)) {
        return 0;
    }
    return 1;
}

void z80_swap_reg(uint8_t* reg_a, uint8_t* reg_b) {
    uint8_t tmp = *reg_a;
    *reg_a = *reg_b;
    *reg_b = tmp;
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
    uint8_t operand_1, operand_2, operand_3, operand_4;
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

        s_reg = ((z80_state.gp[reg_H] << 8) | z80_state.gp[reg_L]);

        if (!z80_ram_valid(s_reg)) {
            printf("Unkown source RAM address %u for LD instruction\n",
                    s_reg);
        }
        z80_state.gp[t_reg] = z80_state.ram[s_reg];

    /* LD r, (IY+d) */
    } else if (operand_1 == 0xFD) {
        operand_2 = z80_fetch_byte();
        t_reg = z80_get_t_reg(operand_2);
        operand_3 = z80_fetch_byte();

        /* Cast operand_3 to signed because it's in twos complement */
        z80_state.gp[t_reg] = (z80_state.iy + (int8_t)operand_3);

    /* LD (HL), r */
    } else if ((operand_1 & 0xF8) == 0x70) {
        t_reg = ((z80_state.gp[reg_H] << 8) | z80_state.gp[reg_L]);

        if(!z80_ram_valid(t_reg)) {
            printf("Unknown target RAM address %d for LD instruction\n",
                    t_reg);
        }
        z80_state.ram[t_reg] = z80_get_s_reg(operand_1);

    } else if (operand_1 == 0xDD) {
        operand_2 = z80_fetch_byte();

        if ((operand_2 & 0xC7) == 0x46) {
            operand_2 = z80_fetch_byte();
            t_reg = z80_get_t_reg(operand_2);
            operand_3 = z80_fetch_byte();

            z80_state.gp[t_reg] = (z80_state.ix + (int8_t)operand_3);

        /* LD (IX+d), r */
        } else if ((operand_2 & 0xF8) == 0x70) {
            s_reg = z80_get_s_reg(operand_2);
            operand_3 = z80_fetch_byte();
            t_reg = z80_state.ix + (int8_t)operand_3;

            if(!z80_ram_valid(t_reg)) {
                printf("Unknown target RAM address %d for LD instruction\n",
                    t_reg);
            }
            z80_state.ram[t_reg] = z80_state.gp[s_reg];

        /* LD (IX+d), n */
        } else if (operand_2 == 0x36) {
            operand_3 = z80_fetch_byte();
            operand_4 = z80_fetch_byte();
            t_reg = z80_state.ix + (int8_t)operand_3;

            if(!z80_ram_valid(t_reg)) {
                printf("Unknown target RAM address %d for LD instruction\n",
                    t_reg);
            }
            z80_state.ram[t_reg] = operand_4;

        /* LD IX, nn */
        } else if (operand_2 == 0x21) {
            operand_3 = z80_fetch_byte();
            operand_4 = z80_fetch_byte();
            z80_state.ix = ((operand_4 << 8) | operand_3);

        /* LD IX, (nn) */
        } else if (operand_2 == 0x2A) {
            operand_3 = z80_fetch_byte();
            operand_4 = z80_fetch_byte();
            s_reg = ((operand_4 << 8) | operand_3);

            if(!z80_ram_valid(s_reg)) {
                printf("Unknown target RAM address %d for LD instruction\n",
                    s_reg);
            }

            /* Get nn+1 to get the second value for the high 8bits of IX */
            t_reg = s_reg + 1;

            /* Check if the loaded address is valid position in ram */
            if(!z80_ram_valid(t_reg)) {
                printf("Unknown target RAM address %d for LD instruction\n",
                    s_reg);
            }

            z80_state.ix = (z80_state.ram[t_reg] << 8) | z80_state.ram[s_reg];

        /* LD (nn), IX */
        } else if (operand_2 == 0x22) {
            operand_3 = z80_fetch_byte();
            operand_4 = z80_fetch_byte();

            s_reg = ((operand_4 << 8) | operand_3);

            if(!z80_ram_valid(s_reg)) {
                printf("Unknown target RAM address %d for LD instruction\n",
                    s_reg);
            }

            /* Get nn+1 */
            t_reg = s_reg + 1;

            /* Check if the loaded address is valid position in ram */
            if(!z80_ram_valid(t_reg)) {
                printf("Unknown target RAM address %d for LD instruction\n",
                    s_reg);
            }
            /* Mask out upper 8bits of the ix and write to memory */
            z80_state.ram[s_reg] = (uint8_t) z80_state.ix;
            /* Shift high bits to right and write to memory */
            z80_state.ram[t_reg] = (uint8_t) (z80_state.ix >> 8);

        /* LD SP, IX */
        } else if (operand_2 == 0xF9) {
            z80_state.sp = z80_state.ix;

        /* PUSH IX */
        } else if (operand_2 == 0xE5) {
            z80_state.sp--;

            if(!z80_ram_valid(z80_state.sp)) {
                printf("Unknown target RAM address %d for PUSH IX \
                        instruction\n", z80_state.sp);
            }

            z80_state.ram[z80_state.sp] = (uint8_t) (z80_state.ix >> 8);
            z80_state.sp--;

            if(!z80_ram_valid(z80_state.sp)) {
                printf("Unknown target RAM address %d for PUSH IX \
                        instruction\n", z80_state.sp);
            }
            z80_state.ram[z80_state.sp] = (uint8_t) z80_state.ix;

        /* POP IX */
        } else if (operand_2 == 0xE1) {
            if(!z80_ram_valid(z80_state.sp)) {
                printf("Unknown target RAM address %d for POP IX \
                        instruction\n", z80_state.sp);
            }

            /* Load low order byte of IX with value of sp position */
            z80_state.ix = z80_state.ram[z80_state.sp];
            z80_state.sp++;

            if(!z80_ram_valid(z80_state.sp)) {
                printf("Unknown target RAM address %d for POP IX \
                        instruction\n", z80_state.sp);
            }

            /* load high order byte of IX with value of sp position */
            z80_state.ix = z80_state.ix | (z80_state.ram[z80_state.sp] << 8);
            z80_state.sp++;

        /* EX (SP), IX */
        } else if (operand_2 == 0xE3) {
            s_reg = z80_state.sp;
            t_reg = z80_state.sp+1;

            if(!z80_ram_valid(s_reg)) {
                printf("Unknown target RAM address %d for PUSH qq \
                        instruction\n", s_reg);
            }

            if(!z80_ram_valid(t_reg)) {
                printf("Unknown target RAM address %d for PUSH qq \
                        instruction\n", t_reg);
            }

            /*Missuse operands 3 and 4 to store immediate values*/
            operand_3 = (uint8_t) z80_state.ix;
            operand_4 = (uint8_t) (z80_state.ix >> 8);

            z80_swap_reg(&z80_state.ram[s_reg], &operand_3);
            z80_swap_reg(&z80_state.ram[t_reg], &operand_4);

        /* ADD A, (IX+d) */
        } else if (operand_2 == 0x86) {
            /* TODO: Set condition flags correctly */
        }
    } else if(operand_1 == 0xFD) {
        operand_2 = z80_fetch_byte();
        /* LD (IY+d), r*/
        if((operand_2 & 0xF8) == 0x70) {
            s_reg = z80_get_s_reg(operand_2);
            operand_3 = z80_fetch_byte();
            t_reg = z80_state.iy + (int8_t)operand_3;

            if(!z80_ram_valid(t_reg)) {
                printf("Unknown target RAM address %d for LD instruction\n",
                    t_reg);
            }
            z80_state.ram[t_reg] = z80_state.gp[s_reg];
        /* LD (IY+d), n */
        } else if (operand_2 == 0x36) {
            operand_3 = z80_fetch_byte();
            operand_4 = z80_fetch_byte();
            t_reg = z80_state.iy + (int8_t)operand_3;

            if(!z80_ram_valid(t_reg)) {
                printf("Unknown target RAM address %d for LD instruction\n",
                    t_reg);
            }
            z80_state.ram[t_reg] = operand_4;

        /* LD IY, nn */
        } else if (operand_2 == 0x21) {
            operand_3 = z80_fetch_byte();
            operand_4 = z80_fetch_byte();
            z80_state.iy = ((operand_4 << 8) | operand_3);

        /* LD IY, (nn) */
        } else if (operand_2 == 0x2A) {
             operand_3 = z80_fetch_byte();
            operand_4 = z80_fetch_byte();
            s_reg = ((operand_4 << 8) | operand_3);

            if(!z80_ram_valid(s_reg)) {
                printf("Unknown target RAM address %d for LD instruction\n",
                    s_reg);
            }

            /* Get nn+1 to get the second value for the high 8bits of IX */
            t_reg = s_reg + 1;

            /* Check if the loaded address is valid position in ram */
            if(!z80_ram_valid(t_reg)) {
                printf("Unknown target RAM address %d for LD instruction\n",
                    s_reg);
            }

            z80_state.iy = (z80_state.ram[t_reg] << 8) | z80_state.ram[s_reg];

        /* LD (nn), IY */
        } else if (operand_2 == 0x22) {
            operand_3 = z80_fetch_byte();
            operand_4 = z80_fetch_byte();

            s_reg = ((operand_4 << 8) | operand_3);

            if(!z80_ram_valid(s_reg)) {
                printf("Unknown target RAM address %d for LD instruction\n",
                    s_reg);
            }

            /* Get nn+1 */
            t_reg = s_reg + 1;

            /* Check if the loaded address is valid position in ram */
            if(!z80_ram_valid(t_reg)) {
                printf("Unknown target RAM address %d for LD instruction\n",
                    s_reg);
            }
            /* Mask out upper 8bits of the ix and write to memory */
            z80_state.ram[s_reg] = (uint8_t) z80_state.iy;
            /* Shift high bits to right and write to memory */
            z80_state.ram[t_reg] = (uint8_t) (z80_state.iy >> 8);

        /* LD SP, IY */
        } else if (operand_2 == 0xF9) {
            z80_state.sp = z80_state.iy;

        /* PUSH IY */
        } else if (operand_2 == 0xE5) {
            z80_state.sp--;

            if(!z80_ram_valid(z80_state.sp)) {
                printf("Unknown target RAM address %d for PUSH IX \
                        instruction\n", z80_state.sp);
            }

            z80_state.ram[z80_state.sp] = (uint8_t) (z80_state.ix >> 8);
            z80_state.sp--;

            if(!z80_ram_valid(z80_state.sp)) {
                printf("Unknown target RAM address %d for PUSH IX \
                        instruction\n", z80_state.sp);
            }
            z80_state.ram[z80_state.sp] = (uint8_t) z80_state.iy;

        /* POP IY */
        } else if (operand_2 == 0xE1) {
            if(!z80_ram_valid(z80_state.sp)) {
                printf("Unknown target RAM address %d for POP IX \
                        instruction\n", z80_state.sp);
            }

            /* Load low order byte of IX with value of sp position */
            z80_state.iy = z80_state.ram[z80_state.sp];
            z80_state.sp++;

            if(!z80_ram_valid(z80_state.sp)) {
                printf("Unknown target RAM address %d for POP IX \
                        instruction\n", z80_state.sp);
            }

            /* load high order byte of IX with value of sp position */
            z80_state.iy = z80_state.iy | (z80_state.ram[z80_state.sp] << 8);
            z80_state.sp++;
        }
    /* LD (HL), n */
    } else if (operand_1 == 0x36) {
        operand_2 = z80_fetch_byte();
        t_reg = ((z80_state.gp[reg_H] << 8) | z80_state.gp[reg_L]);

        if(!z80_ram_valid(t_reg)) {
            printf("Unknown target RAM address %d for LD instruction\n",
                    t_reg);
        }
        z80_state.ram[t_reg] = operand_2;

    /* LD A, (BC) */
    } else if (operand_1 == 0x0A) {
        s_reg = ((z80_state.gp[reg_B] << 8) | z80_state.gp[reg_C]);
        if(!z80_ram_valid(s_reg)) {
            printf("Unknown target RAM address %d for LD instruction\n",
                    s_reg);
        }
        /* Load memory from BC into accumulator */
        z80_state.acc = z80_state.ram[s_reg];

    /* LD A, (DE) */
    } else if (operand_1 == 0x1A) {
        s_reg = ((z80_state.gp[reg_D] << 8) | z80_state.gp[reg_E]);
        if(!z80_ram_valid(s_reg)) {
            printf("Unknown target RAM address %d for LD instruction\n",
                    s_reg);
        }
        /* Load memory from BC into accumulator */
        z80_state.acc = z80_state.ram[s_reg];
    /* LD A, (nn) */
    } else if (operand_1 == 0x3A) {
        operand_3 = z80_fetch_byte();
        operand_4 = z80_fetch_byte();
        t_reg = ((operand_4 << 8) | operand_3);

        /* Check if the loaded address is valid position in ram */
        if(!z80_ram_valid(t_reg)) {
            printf("Unknown target RAM address %d for LD instruction\n",
                    t_reg);
        }

        /* Load accumulator content into ram position */
        z80_state.ram[t_reg] = z80_state.acc;

    /* LD (BC), A */
    } else if (operand_1 == 0x02) {
        t_reg = ((z80_state.gp[reg_B] << 8) | z80_state.gp[reg_C]);

        if(!z80_ram_valid(t_reg)) {
            printf("Unknown target RAM address %d for LD (BC), A \
                    instruction\n", t_reg);
        }
        z80_state.ram[t_reg] = z80_state.acc;

    /* LD (DE), A */
    } else if (operand_1 == 0x12) {
        t_reg = ((z80_state.gp[reg_D] << 8) | z80_state.gp[reg_E]);

        if(!z80_ram_valid(t_reg)) {
            printf("Unknown target RAM address %d for LD (DE), A \
                    instruction\n", t_reg);
        }
        z80_state.ram[t_reg] = z80_state.acc;

    /* LD (nn), A */
    } else if (operand_1 == 0x32) {
        operand_3 = z80_fetch_byte();
        operand_4 = z80_fetch_byte();
        s_reg = ((operand_4 << 8) | operand_3);

        /* Check if the loaded address is valid position in ram */
        if(!z80_ram_valid(s_reg)) {
            printf("Unknown target RAM address %d for LD (nn), A \
                    instruction\n", s_reg);
        }

        /* Load conent from ram position into accumulator */
        z80_state.acc = z80_state.ram[s_reg];

    } else if (operand_1 == 0xED) {
        operand_2 = z80_fetch_byte();
        /* LD A, I */
        if (operand_2 == 0x57) {
            z80_state.acc = z80_state.i;
            z80_update_flags(z80_state.i, SIGN_FLAG | ZERO_FLAG |
                    HALFCARRY_FLAG | PARITYOVERFLOW_FLAG | ADDSUB_FLAG);

        /* LD A, R */
        } else if (operand_2 == 0x5F) {
            z80_state.acc = z80_state.r;
            z80_update_flags(z80_state.i, SIGN_FLAG | ZERO_FLAG |
                    HALFCARRY_FLAG | PARITYOVERFLOW_FLAG | ADDSUB_FLAG);

        /* LD I, A */
        } else if (operand_2 == 0x47) {
            z80_state.i = z80_state.acc;

        /* LD R, A */
        } else if (operand_2 == 0x4F) {
            z80_state.r = z80_state.acc;

        /* LD dd, (nn) */
        } else if ((operand_2 & 0xCF) == 0x4B) {
        /* LD (nn), dd */
        } else if ((operand_2 & 0xCF) == 0x43) {
        /* LDI */
        } else if (operand_2 == 0xA0) {
        /* LDIR */
        } else if (operand_2 == 0xB0) {
        /* LDD */
        } else if (operand_2 == 0xA8) {
        /* LDDR */
        } else if (operand_2 == 0xB8) {
        /* CPI */
        } else if (operand_2 == 0xA1) {
        /* CPIR */
        } else if (operand_2 == 0xB1) {
        /* CPD */
        } else if (operand_2 == 0xA9) {
        /* CPDR */
        } else if (operand_2 == 0xB9) {
        }
    /* LD dd, nn */
    } else if ((operand_1 & 0xCF) == 0x1) {
        operand_2 = z80_fetch_byte();
        operand_3 = z80_fetch_byte();

        switch((operand_1 & 0x30) >> 4) {
        case 0: /*BC*/
            z80_state.gp[reg_B] = operand_2;
            z80_state.gp[reg_C] = operand_3;
            break;
        case 1: /*DE*/
            z80_state.gp[reg_D] = operand_2;
            z80_state.gp[reg_E] = operand_3;
            break;
        case 2: /*HL*/
            z80_state.gp[reg_H] = operand_2;
            z80_state.gp[reg_L] = operand_3;
            break;
        case 3: /*SP*/
            z80_state.sp  = (operand_3 << 8) | operand_2;
            break;
        }

    /* LD HL, (nn) */
    } else if (operand_1 == 0x2A) {
        operand_3 = z80_fetch_byte();
        operand_4 = z80_fetch_byte();

        s_reg = ((operand_4 << 8) | operand_3);

        if(!z80_ram_valid(s_reg)) {
            printf("Unknown target RAM address %d for LD HL, (nn) \
                    instruction\n", s_reg);
        }
        z80_state.gp[reg_L] = z80_state.ram[s_reg];

        /* Get nn+1 */
        s_reg++;

        /* Check if the loaded address is valid position in ram */
        if(!z80_ram_valid(s_reg)) {
            printf("Unknown target RAM address %d for LD HL, (nn) \
                    instruction\n", s_reg);
        }
        z80_state.gp[reg_H] = z80_state.ram[s_reg];

    /* LD (nn), HL */
    } else if (operand_1 == 0x22) {
        operand_3 = z80_fetch_byte();
        operand_4 = z80_fetch_byte();

        t_reg = ((operand_4 << 8) | operand_3);

        if(!z80_ram_valid(t_reg)) {
            printf("Unknown target RAM address %d for LD (nn), HL \
                    instruction\n", t_reg);
        }
        z80_state.ram[t_reg] = z80_state.gp[reg_L];

        /* Get nn+1 */
        t_reg++;

        /* Check if the loaded address is valid position in ram */
        if(!z80_ram_valid(t_reg)) {
            printf("Unknown target RAM address %d for LD (nn), HL \
                    instruction\n", t_reg);
        }
        z80_state.ram[t_reg] = z80_state.gp[reg_H];

    /* LD SP, HL */
    } else if (operand_1 == 0xF9) {
        z80_state.sp = (z80_state.gp[reg_H] << 8) | z80_state.gp[reg_L];

    /* PUSH qq */
    } else if ((operand_1 & 0xCF) == 0xC5) {
        s_reg = --z80_state.sp;
        t_reg = --z80_state.sp;

        if(!z80_ram_valid(s_reg)) {
            printf("Unknown target RAM address %d for PUSH qq \
                    instruction\n", s_reg);
        }

        if(!z80_ram_valid(t_reg)) {
            printf("Unknown target RAM address %d for PUSH qq \
                    instruction\n", t_reg);
        }

        switch((operand_1 & 0x30) >> 4) {
        case 0: /*BC*/
            z80_state.ram[s_reg] = z80_state.gp[reg_B];
            z80_state.ram[t_reg] = z80_state.gp[reg_C];
            break;
        case 1: /*DE*/
            z80_state.ram[s_reg] = z80_state.gp[reg_D];
            z80_state.ram[t_reg] = z80_state.gp[reg_E];
            break;
        case 2: /*HL*/
            z80_state.ram[s_reg] = z80_state.gp[reg_H];
            z80_state.ram[t_reg] = z80_state.gp[reg_L];
            break;
        case 3: /*AF*/
            z80_state.ram[s_reg] = z80_state.acc;
            z80_state.ram[t_reg] = z80_state.flags;
            break;
        }

    /* POP qq */
    } else if ((operand_1 & 0xCF) == 0xC1) {
        s_reg = z80_state.sp++;
        t_reg = z80_state.sp++;

        if(!z80_ram_valid(s_reg)) {
            printf("Unknown target RAM address %d for PUSH qq \
                    instruction\n", s_reg);
        }

        if(!z80_ram_valid(t_reg)) {
            printf("Unknown target RAM address %d for PUSH qq \
                    instruction\n", t_reg);
        }

        switch((operand_1 & 0x30) >> 4) {
        case 0: /*BC*/
            z80_state.gp[reg_B] = z80_state.ram[s_reg];
            z80_state.gp[reg_C] = z80_state.ram[t_reg];
            break;
        case 1: /*DE*/
            z80_state.gp[reg_D] = z80_state.ram[s_reg];
            z80_state.gp[reg_E] = z80_state.ram[t_reg];
            break;
        case 2: /*HL*/
            z80_state.gp[reg_H] = z80_state.ram[s_reg];
            z80_state.gp[reg_L] = z80_state.ram[t_reg];
            break;
        case 3: /*AF*/
            z80_state.acc = z80_state.ram[s_reg];
            z80_state.flags = z80_state.ram[t_reg];
            break;
        }

    /* Block Transfer and Search */
    /* EX DE, HL */
    } else if(operand_1 == 0xEB) {

        z80_swap_reg(&z80_state.gp[reg_D], &z80_state.gp[reg_H]);
        z80_swap_reg(&z80_state.gp[reg_E], &z80_state.gp[reg_L]);

    /* EX AF, AF' */
    } else if(operand_1 == 0x08) {
        z80_swap_reg(&z80_state.acc, &z80_state.acc_);
        z80_swap_reg(&z80_state.flags, &z80_state.flags_);

    /* EXX */
    } else if(operand_1 == 0xD9) {
        z80_swap_reg(&z80_state.gp[reg_B], &z80_state.gp[reg_B_]);
        z80_swap_reg(&z80_state.gp[reg_C], &z80_state.gp[reg_C_]);
        z80_swap_reg(&z80_state.gp[reg_D], &z80_state.gp[reg_D_]);
        z80_swap_reg(&z80_state.gp[reg_E], &z80_state.gp[reg_E_]);
        z80_swap_reg(&z80_state.gp[reg_H], &z80_state.gp[reg_H_]);
        z80_swap_reg(&z80_state.gp[reg_L], &z80_state.gp[reg_L_]);

    /* EX (SP), HL */
    } else if(operand_1 == 0xE3) {
        s_reg = z80_state.sp;
        t_reg = z80_state.sp+1;

        if(!z80_ram_valid(s_reg)) {
            printf("Unknown target RAM address %d for PUSH qq \
                    instruction\n", s_reg);
        }

        if(!z80_ram_valid(t_reg)) {
            printf("Unknown target RAM address %d for PUSH qq \
                    instruction\n", t_reg);
        }

        z80_swap_reg(&z80_state.ram[s_reg], &z80_state.gp[reg_L]);
        z80_swap_reg(&z80_state.ram[t_reg], &z80_state.gp[reg_H]);

    /* EX (SP), IY */
    } else if(1) {

    /* Arithmetic and Logical */
    /* ADD A, r */
    } else if((operand_1 & 0x07) == 0x80) {
        /* TODO: Set condition flags correctly */
        s_reg = z80_get_s_reg(operand_1);
        z80_state.acc += s_reg;

    /* ADD A, n */
    } else if(operand_1 == 0xC6) {
        /* TODO: Set condition flags correctly */
        operand_2 = z80_fetch_byte();
        z80_state.acc += operand_2;

    /* ADD A, (HL) */
    } else if(operand_1 == 0x86) {
        /* TODO: Set condition flags correctly */
        s_reg = (z80_state.gp[reg_H] << 8) | z80_state.gp[reg_L];

        if(!z80_ram_valid(s_reg)) {
            printf("Unknown target RAM address %d for PUSH qq \
                    instruction\n", s_reg);
        }
        z80_state.acc += z80_state.ram[s_reg];

    /* ADD A, (IY+d) */
    } else if(1) {
    /* ADC A, s */
    } else if(1) {
    /* SUB r */
    } else if((operand_1 & 0xF8) ==  0x90) {
        /* TODO: Set condition flags correctly */
        s_reg = z80_get_s_reg(operand_1);
        z80_state.acc -= s_reg;

    /* SUB n */
    } else if (operand_1 == 0xD6) {
        /* TODO: Set condition flags correctly */
        operand_2 = z80_fetch_byte();
        z80_state.acc -= operand_2;

    /* SBC A, s */
    } else if(1) {
    /* AND s */
    } else if(1) {
    /* OR s */
    } else if(1) {
    /* XOR s */
    } else if(1) {
    /* CP s*/
    } else if(1) {
    /* INC r */
    } else if(1) {
    /* INC (HL) */
    } else if(1) {
    /* INC (IX+d) */
    } else if(1) {
    /* INC (IY+d) */
    } else if(1) {
    /* DEC m */
    } else if(1) {
    /* DAA */
    } else if(1) {
    /* CPL */
    } else if(1) {
    /* NEG */
    } else if(1) {
    /* CCF */
    } else if(1) {
    /* SCF */
    } else if(1) {
    /* NOP */
    } else if(1) {
    /* HALT */
    } else if(1) {
    /* DI */
    } else if(1) {
    /* EI */
    } else if(1) {
    /* IM 0 */
    } else if(1) {
    /* IM 1 */
    } else if(1) {
    /* IM 2 */

    /* ADD HL, ss */
    } else if(1) {
    /* ADC HL, ss */
    } else if(1) {
    /* SBC HL, ss */
    } else if(1) {
    /* ADD IX, pp */
    } else if(1) {
    /* ADD IY, rr */
    } else if(1) {
    /* INC ss */
    } else if(1) {
    /* INC IX */
    } else if(1) {
    /* INC IY */
    } else if(1) {
    /* DEC ss */
    } else if(1) {
    /* DEC IX */
    } else if(1) {
    /* DEC IY */
    } else if(1) {

    /* Rotate and Shift */
    /* RLCA */
    } else if(1) {
    /* RLA */
    } else if(1) {
    /* RLCA */
    } else if(1) {
    /* RRA */
    } else if(1) {
    /* RLC r */
    } else if(1) {
    /* RLC (HL) */
    } else if(1) {
    /* RLC (IX+d) */
    } else if(1) {
    /* RLC (IY+d) */
    } else if(1) {
    /* RL m */
    } else if(1) {
    /* RRC m */
    } else if(1) {
    /* RR m */
    } else if(1) {
    /* SLA m */
    } else if(1) {
    /* SRA m */
    } else if(1) {
    /* SRL m */
    } else if(1) {
    /* RLD */
    } else if(1) {
    /* RRD */
    } else if(1) {

    /* Bit Manipulation (Set, Reset, Test) */
    /* BIT b, r */
    } else if(1) {
    /* BIT b, (HL) */
    } else if(1) {
    /* BIT b, (IX+d) */
    } else if(1) {
    /* BIT b, (IY+d) */
    } else if(1) {
    /* SET b, r */
    } else if(1) {
    /* SET b, (HL) */
    } else if(1) {
    /* SET b, (IX+d) */
    } else if(1) {
    /* SET b, (IY+d) */
    } else if(1) {
    /* RES b, m */
    } else if(1) {

    /* Jump, Call, and Return */
    /* JP nn */
    } else if(operand_1 == 0xC3) {
        operand_2 = z80_fetch_byte();
        operand_3 = z80_fetch_byte();

        s_reg = ((operand_3 << 8) | operand_2);

        if(!z80_ram_valid(s_reg)) {
            printf("Unknown target RAM address %d for JP nn \
                    instruction\n", s_reg);
        }
        z80_state.pc = z80_state.ram[s_reg];

    /* JP cc, nn */
    } else if((operand_1 & 0xC7) == 0xC2) {
        operand_2 = z80_fetch_byte();
        operand_3 = z80_fetch_byte();

        if((operand_1 & 0x38 >> 3) ) {
            s_reg = ((operand_3 << 8) | operand_2);

            if(!z80_ram_valid(s_reg)) {
                printf("Unknown target RAM address %d for JP nn \
                        instruction\n", s_reg);
            }
            z80_state.pc = z80_state.ram[s_reg];
        }
    /* JR e */
    } else if(1) {
    /* JR C, e */
    } else if(1) {
    /* JR NC, e */
    } else if(1) {
    /* JR Z, e */
    } else if(1) {
    /* JR NZ, e */
    } else if(1) {
    /* JP (HL) */
    } else if(1) {
    /* JP (IX) */
    } else if(1) {
    /* JP (IY) */
    } else if(1) {
    /* DJNZ, e */
    } else if(1) {
    /* CALL nn */
    } else if(1) {
    /* CALL cc, nn */
    } else if(1) {
    /* RET */
    } else if(1) {
    /* RET cc */
    } else if(1) {
    /* RETI */
    } else if(1) {
    /* RETN */
    } else if(1) {
    /*RST p */
    } else if(1) {

    /* Input/Output */
    /* IN A, (n) */
    } else if(1) {
    /* IN r (C)*/
    } else if(1) {
    /* INI */
    } else if(1) {
    /* INIR */
    } else if(1) {
    /* IND */
    } else if(1) {
    /* INDR */
    } else if(1) {
    /* OUT (n), A */
    } else if(1) {
    /* OUT (C), r */
    } else if(1) {
    /* OUTI */
    } else if(1) {
    /* OTIR */
    } else if(1) {
    /* OUTD */
    } else if(1) {
    /* OTDR */
    } else if(1) {
    }
}

