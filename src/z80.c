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
#include <stdlib.h>

#include "encodings.h"
#include "z80.h"
#include "loader.h"

uint8_t* rom_handle;

/***
* Struct which holds the Z80's processor state
*/
struct z80_vCPU {
  uint8_t gp[12]; /* General Purpose Registers */
  uint8_t acc;  /* Accumulator */
  uint8_t flags;  /* Flags register */
  uint8_t acc_;   /* A' */
  uint8_t flags_; /* F' */
  uint8_t r;    /* Memory Refresh */
  uint8_t i;    /* Interrupt Vector */
  uint16_t ix;  /* Index Register */
  uint16_t iy;  /* Index Register */
  uint16_t sp;  /* Stack Pointer */
  uint16_t pc;  /* Program Counter */
  uint8_t iff1;   /*Interrupt Enable/Disable Flip-Flop I*/
  uint8_t iff2;   /*Interrupt Enable/Disable Flip-Flop II*/
};

struct z80_state {
  struct z80_vCPU vcpu;
  uint8_t stack[256];
  uint8_t ram[RAM_SZ];  /* 8KB RAM */
  uint8_t vram[VRAM_SZ]; /* 16KB VRAM */
} z80_state;

const char* z80_decode_gp_reg(uint16_t enc) {
  switch(enc) {
  case A: return "A";
  case B: return "B";
  case C: return "C";
  case D: return "D";
  case E: return "E";
  case H: return "H";
  case L: return "L";
  default: return "UNKNOWN!";
  }
  return "UNKNOWN!";
}

void dump_stack() {
  uint16_t i;

  printf("TOP of STACK:\n-----------------------------------");
  for(i = 255; i > 191; i--) {
    if((i % 16) == 15)
      printf("\n%04x: ", i);
    if(z80_state.vcpu.sp == i)
      printf("[%02x] ", z80_state.stack[i]);
    else
      printf(" %02x  ", z80_state.stack[i]);
  }
  printf("\n-----------------------------------\n\n");
}

void z80_emulate_cycle(void) {
  /* Fetch OpCode */
  z80_decode_insn();
  /* Decode OpCode */
  /* Execute OpCode */
  /* Update Timers */
  return;
}

void z80_init(const char* rom_path) {
  /* Set PC to first address in RAM */
  rom_handle = (uint8_t*)malloc(512 * 1024 * sizeof(uint8_t));
  loader_load_rom(rom_path, rom_handle);
  z80_state.vcpu.pc = 0x8000;
  z80_state.vcpu.sp = 255;
  return;
}

int z80_flag_set(uint8_t mask, uint8_t flag) {
  if ((mask & flag) == flag)
    return 1;
  return 0;
}

int z80_condition_true(uint8_t cc) {
  switch(cc) {
  case 0x0: /*NZ non zero*/
    return !(z80_state.vcpu.flags & ZERO_FLAG);
  case 0x1: /*Z zero*/
    return (z80_state.vcpu.flags & ZERO_FLAG);
  case 0x2: /*NC no carry*/
    return !(z80_state.vcpu.flags & CARRY_FLAG);
  case 0x3: /*C carry*/
    return (z80_state.vcpu.flags & CARRY_FLAG);
  case 0x4: /*PO parity odd*/
    return !(z80_state.vcpu.flags & PARITYOVERFLOW_FLAG);
  case 0x5: /*PE parity even*/
    return (z80_state.vcpu.flags & PARITYOVERFLOW_FLAG);
  case 0x6: /*P sign positive*/
    return !(z80_state.vcpu.flags & SIGN_FLAG);
  case 0x7: /*M sign negative*/
    return (z80_state.vcpu.flags & SIGN_FLAG);
  }
  return 0;
}

void z80_set_carry_flag() {
  z80_state.vcpu.flags |= CARRY_FLAG;
}

void z80_clear_carry_flag() {
  z80_state.vcpu.flags &= ~CARRY_FLAG;
}

void z80_update_flags(uint8_t value, uint8_t mask) {
  if(z80_flag_set(mask, SIGN_FLAG)) {
    /* S is set if result is negative; reset otherwise*/
    if((value & 0x80) == 0x80)
      z80_state.vcpu.flags |= SIGN_FLAG;
    else
      z80_state.vcpu.flags &= ~SIGN_FLAG;
  }

  if(z80_flag_set(mask, ZERO_FLAG)) {
    /* Z is set if result is zero; reset otherwise */
    if (value == 0)
      z80_state.vcpu.flags |= ZERO_FLAG;
    else
      z80_state.vcpu.flags &= ~ZERO_FLAG;
  }

  if(z80_flag_set(mask, HALFCARRY_FLAG)) {
    /* H is set if carry from bit 3; reset otherwise */
    if((value & 0x10) == 0x10)
      z80_state.vcpu.flags |= HALFCARRY_FLAG;
    else
      z80_state.vcpu.flags &= ~HALFCARRY_FLAG;
  }

  if(z80_flag_set(mask, ADDSUB_FLAG)) {
    /* N is reset */
    z80_state.vcpu.flags &= ~ADDSUB_FLAG;
  }

  if(z80_flag_set(mask, PARITYOVERFLOW_FLAG)) {
    /* Store state of iff2 in parity flag*/
    if(z80_state.vcpu.iff2)
      z80_state.vcpu.flags |= PARITYOVERFLOW_FLAG;
    else
      z80_state.vcpu.flags &= ~PARITYOVERFLOW_FLAG;
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
    printf("Unknown target register %u\n", t_reg);
    return 0;
  }
  return t_reg;
}

uint8_t z80_get_s_reg(uint8_t operand) {
  uint8_t s_reg = (operand & 0x7);
  if (!z80_gp_valid(s_reg)) {
    printf("Unknown source register %u\n", s_reg);
    return 0;
  }
  return s_reg;
}

/*TODO: Fix this check for real ram size*/
int z80_ram_valid(uint16_t value) {
  if (value >= RAM_SZ) {
    return 1;
  }
  return 1;
}

uint8_t z80_fetch_byte(void) {
  /* Check if PC points to valid position */
  //if (!z80_ram_valid(z80_state.vcpu.pc)) {
  //  printf("Unknown PC position %u\n", z80_state.vcpu.pc);
  //  return 0;
  //}
  /* Increment PC after returning instruction */
  return rom_handle[z80_state.vcpu.pc++];
}

/***
 * The Z80 CPU can execute 158 different instruction types including all 78 of
 * the 8080A CPU
 */
void z80_decode_insn() {
  uint8_t operand_1, operand_2, operand_3, operand_4;
  uint16_t s_reg = 0;
  uint16_t t_reg = 0;
  uint16_t tmp;

  operand_1 = z80_fetch_byte();
#ifdef DEBUG
  printf("0x%04x:\t0x%02x\t", z80_state.vcpu.pc, operand_1);
#endif

  /* LD r, r' */
  if ((operand_1 & 0xC0) == 0x40) {
    /* Get source and destination registers */
    s_reg = z80_get_s_reg(operand_1);
    t_reg = z80_get_t_reg(operand_1);

    /* Store value of source register in target register*/
    z80_state.vcpu.gp[t_reg] = z80_state.vcpu.gp[s_reg];
#ifdef DEBUG
    printf("LD %s, %s'\n", z80_decode_gp_reg(t_reg), z80_decode_gp_reg(s_reg));
#endif

  /* LD r, n */
  } else if ((operand_1 & 0xC7) == 0x06) {
    /* Load second operand from memory */
    operand_2 = z80_fetch_byte();

    /* Determine target register */
    t_reg = z80_get_t_reg(operand_1);

    /* Store operand_2 in target register */
    z80_state.vcpu.gp[t_reg] = operand_2;
#ifdef DEBUG
    printf("LD %s, 0x%0x\n", z80_decode_gp_reg(t_reg), operand_2);
#endif

  /* LD r, (HL) */
  } else if ((operand_1 & 0xC7) == 0x46) {

    t_reg = z80_get_t_reg(operand_1);

    s_reg = ((z80_state.vcpu.gp[reg_H] << 8) | z80_state.vcpu.gp[reg_L]);

    if (!z80_ram_valid(s_reg)) {
      printf("Unkown source RAM address %u for LD instruction\n", s_reg);
    }
    z80_state.vcpu.gp[t_reg] = z80_state.ram[s_reg];
#ifdef DEBUG
    printf("LD r, (HL)\n");
#endif

  } else if (operand_1 == 0xDD) {
    operand_2 = z80_fetch_byte();
    operand_3 = z80_fetch_byte();

    /* LD r, (IX+d) */
    if((operand_2 & 0xC7) == 0x46) {
      t_reg = z80_get_t_reg(operand_2);
      z80_state.vcpu.gp[t_reg] = z80_state.vcpu.ix + operand_3;
#ifdef DEBUG
      printf("LD %s, (0x%0x + 0x%0x)\n", z80_decode_gp_reg(t_reg), z80_state.vcpu.ix, operand_3);
#endif
    /* LD (IX+d), r)*/
    } else if((operand_2 & 0xF8) == 0x70) {
      s_reg = z80_get_t_reg(operand_2);
      t_reg = z80_state.vcpu.ix + operand_3;

      if(!z80_ram_valid(t_reg)) {
        printf("Unknown target RAM address 0x%x for LD instruction\n", t_reg);
      }
      z80_state.ram[t_reg] = s_reg;

#ifdef DEBUG
      printf("LD (0x%0x + 0x%0x), %s\n", z80_state.vcpu.ix, operand_3, z80_decode_gp_reg(s_reg));
#endif
    /* LD (IX+d), n */
    } else if (operand_2 == 0x36) {
      operand_4 = z80_fetch_byte();
      tmp = z80_state.vcpu.ix + operand_3;

      if(!z80_ram_valid(t_reg)) {
        printf("Unknown target RAM address 0x%x for LD instruction\n", t_reg);
      }
      z80_state.ram[tmp] = operand_4;
#ifdef DEBUG
      printf("LD (0x%04x), 0x%02x\n", tmp, operand_4);
#endif
    /* LD IX, nn */
    } else if (operand_2 == 0x21) {
      operand_3 = z80_fetch_byte();
      operand_4 = z80_fetch_byte();
      z80_state.vcpu.ix = ((operand_4 << 8) | operand_3);
#ifdef DEBUG
      printf("LD IX, nn\n");
#endif

    /* LD IX, (nn) */
    } else if (operand_2 == 0x2A) {
      operand_3 = z80_fetch_byte();
      operand_4 = z80_fetch_byte();
      s_reg = ((operand_4 << 8) | operand_3);

      if(!z80_ram_valid(s_reg)) {
        printf("Unknown target RAM address 0x%x for LD instruction\n",
          s_reg);
      }

      /* Get nn+1 to get the second value for the high 8bits of IX */
      t_reg = s_reg + 1;

      /* Check if the loaded address is valid position in ram */
      if(!z80_ram_valid(t_reg)) {
        printf("Unknown target RAM address 0x%x for LD instruction\n",
          s_reg);
      }

      z80_state.vcpu.ix = (z80_state.ram[t_reg] << 8) | z80_state.ram[s_reg];
#ifdef DEBUG
      printf("LD IX, (nn)\n");
#endif

    /* LD (nn), IX */
    } else if (operand_2 == 0x22) {
      operand_3 = z80_fetch_byte();
      operand_4 = z80_fetch_byte();

      s_reg = ((operand_4 << 8) | operand_3);

      if(!z80_ram_valid(s_reg)) {
        printf("Unknown target RAM address 0x%x for LD instruction\n",
          s_reg);
      }

      /* Get nn+1 */
      t_reg = s_reg + 1;

      /* Check if the loaded address is valid position in ram */
      if(!z80_ram_valid(t_reg)) {
        printf("Unknown target RAM address 0x%x for LD instruction\n",
          s_reg);
      }
      /* Mask out upper 8bits of the ix and write to memory */
      z80_state.ram[s_reg] = (uint8_t) z80_state.vcpu.ix;
      /* Shift high bits to right and write to memory */
      z80_state.ram[t_reg] = (uint8_t) (z80_state.vcpu.ix >> 8);
#ifdef DEBUG
      printf("LD (nn), IX\n");
#endif

    /* LD SP, IX */
    } else if (operand_2 == 0xF9) {
      z80_state.vcpu.sp = z80_state.vcpu.ix;
#ifdef DEBUG
      printf("LD SP, 0x%0x\n", z80_state.vcpu.ix);
#endif

    /* PUSH IX */
    } else if (operand_2 == 0xE5) {
      z80_state.vcpu.sp--;
      z80_state.stack[z80_state.vcpu.sp] = z80_state.vcpu.ix;
#ifdef DEBUG
      printf("PUSH IX\n");
#endif

    /* POP IX */
    } else if (operand_2 == 0xE1) {
      z80_state.vcpu.ix = z80_state.stack[z80_state.vcpu.sp];
      z80_state.vcpu.sp++;

#ifdef DEBUG
      printf("POP IX\n");
#endif

    /* EX (SP), IX */
    } else if (operand_2 == 0xE3) {
      s_reg = z80_state.vcpu.sp;
      t_reg = z80_state.vcpu.sp+1;

      if(!z80_ram_valid(s_reg)) {
        printf("Unknown target RAM address 0x%x for PUSH qq \
            instruction\n", s_reg);
      }

      if(!z80_ram_valid(t_reg)) {
        printf("Unknown target RAM address 0x%x for PUSH qq \
            instruction\n", t_reg);
      }

      /*Missuse operands 3 and 4 to store immediate values*/
      operand_3 = (uint8_t) z80_state.vcpu.ix;
      operand_4 = (uint8_t) (z80_state.vcpu.ix >> 8);

      z80_swap_reg(&z80_state.ram[s_reg], &operand_3);
      z80_swap_reg(&z80_state.ram[t_reg], &operand_4);
#ifdef DEBUG
      printf("EX (SP), IX\n");
#endif

    /* ADD A, (IX+d) */
    } else if (operand_2 == 0x86) {
      /* TODO: Set condition flags correctly */
#ifdef DEBUG
      printf("ADD A, (IX+d) xxx\n");
#endif

    /* JP (IX) */
    } else if (operand_2 == 0xE9) {
#ifdef DEBUG
      printf("JP (IX) xxx\n");
#endif
    } else {
      printf("Unknown second operand 0x%02x for 0xDD\n", operand_2);
    }
  /* LD r, (IY+d) */
  } else if (operand_1 == 0xFD) {
    operand_2 = z80_fetch_byte();
    operand_3 = z80_fetch_byte();
    t_reg = z80_get_t_reg(operand_2);

    z80_state.vcpu.gp[t_reg] = z80_state.vcpu.iy + operand_3;
#ifdef DEBUG
    printf("LD %s, (0x%0x + 0x%0x)\n", z80_decode_gp_reg(t_reg), z80_state.vcpu.iy, operand_3);
#endif

  /* LD (HL), r */
  } else if ((operand_1 & 0xF8) == 0x70) {
    t_reg = ((z80_state.vcpu.gp[reg_H] << 8) | z80_state.vcpu.gp[reg_L]);

    if(!z80_ram_valid(t_reg)) {
      printf("Unknown target RAM address 0x%x for LD instruction\n",
          t_reg);
    }
    z80_state.ram[t_reg] = z80_get_s_reg(operand_1);
#ifdef DEBUG
    printf("LD (HL), r\n");
#endif

  } else if(operand_1 == 0xFD) {
    operand_2 = z80_fetch_byte();
    /* LD (IY+d), r*/
    if((operand_2 & 0xF8) == 0x70) {
      s_reg = z80_get_s_reg(operand_2);
      operand_3 = z80_fetch_byte();
      t_reg = z80_state.vcpu.iy + (int8_t)operand_3;

      if(!z80_ram_valid(t_reg)) {
        printf("Unknown target RAM address 0x%x for LD instruction\n",
          t_reg);
      }
      z80_state.ram[t_reg] = z80_state.vcpu.gp[s_reg];
#ifdef DEBUG
      printf("LD (IY+d), r\n");
#endif

    /* LD (IY+d), n */
    } else if (operand_2 == 0x36) {
      operand_3 = z80_fetch_byte();
      operand_4 = z80_fetch_byte();
      t_reg = z80_state.vcpu.iy + (int8_t)operand_3;

      if(!z80_ram_valid(t_reg)) {
        printf("Unknown target RAM address 0x%x for LD instruction\n",
          t_reg);
      }
      z80_state.ram[t_reg] = operand_4;

    /* LD IY, nn */
    } else if (operand_2 == 0x21) {
      operand_3 = z80_fetch_byte();
      operand_4 = z80_fetch_byte();
      z80_state.vcpu.iy = ((operand_4 << 8) | operand_3);

    /* LD IY, (nn) */
    } else if (operand_2 == 0x2A) {
       operand_3 = z80_fetch_byte();
      operand_4 = z80_fetch_byte();
      s_reg = ((operand_4 << 8) | operand_3);

      if(!z80_ram_valid(s_reg)) {
        printf("Unknown target RAM address 0x%x for LD instruction\n",
          s_reg);
      }

      /* Get nn+1 to get the second value for the high 8bits of IX */
      t_reg = s_reg + 1;

      /* Check if the loaded address is valid position in ram */
      if(!z80_ram_valid(t_reg)) {
        printf("Unknown target RAM address 0x%x for LD instruction\n",
          s_reg);
      }

      z80_state.vcpu.iy = (z80_state.ram[t_reg] << 8) | z80_state.ram[s_reg];

    /* LD (nn), IY */
    } else if (operand_2 == 0x22) {
      operand_3 = z80_fetch_byte();
      operand_4 = z80_fetch_byte();

      s_reg = ((operand_4 << 8) | operand_3);

      if(!z80_ram_valid(s_reg)) {
        printf("Unknown target RAM address 0x%x for LD instruction\n",
          s_reg);
      }

      /* Get nn+1 */
      t_reg = s_reg + 1;

      /* Check if the loaded address is valid position in ram */
      if(!z80_ram_valid(t_reg)) {
        printf("Unknown target RAM address 0x%x for LD instruction\n",
          s_reg);
      }
      /* Mask out upper 8bits of the ix and write to memory */
      z80_state.ram[s_reg] = (uint8_t) z80_state.vcpu.iy;
      /* Shift high bits to right and write to memory */
      z80_state.ram[t_reg] = (uint8_t) (z80_state.vcpu.iy >> 8);

    /* LD SP, IY */
    } else if (operand_2 == 0xF9) {
      z80_state.vcpu.sp = z80_state.vcpu.iy;
#ifdef DEBUG
      printf("LD SP, IY\n");
#endif

    /* PUSH IY */
    } else if (operand_2 == 0xE5) {
      z80_state.vcpu.sp--;

      z80_state.stack[z80_state.vcpu.sp] = z80_state.vcpu.iy;
#ifdef DEBUG
      printf("PUSH IY\n");
#endif

    /* POP IY */
    } else if (operand_2 == 0xE1) {
      z80_state.vcpu.iy = z80_state.stack[z80_state.vcpu.sp];
      z80_state.vcpu.sp++;
#ifdef DEBUG
      printf("POP IY\n");
#endif
    }
  /* LD (HL), n */
  } else if (operand_1 == 0x36) {
    operand_2 = z80_fetch_byte();
    tmp = ((z80_state.vcpu.gp[reg_H] << 8) | z80_state.vcpu.gp[reg_L]);

    z80_state.ram[t_reg] = operand_2;
#ifdef DEBUG
    printf("LD (0x%04x), 0x%02x\n", t_reg, operand_2);
#endif

  /* LD A, (BC) */
  } else if (operand_1 == 0x0A) {
    s_reg = ((z80_state.vcpu.gp[reg_B] << 8) | z80_state.vcpu.gp[reg_C]);
    if(!z80_ram_valid(s_reg)) {
      printf("Unknown target RAM address 0x%x for LD instruction\n",
          s_reg);
    }
    /* Load memory from BC into accumulator */
    z80_state.vcpu.acc = z80_state.ram[s_reg];

  /* LD A, (DE) */
  } else if (operand_1 == 0x1A) {
    s_reg = ((z80_state.vcpu.gp[reg_D] << 8) | z80_state.vcpu.gp[reg_E]);
    if(!z80_ram_valid(s_reg)) {
      printf("Unknown target RAM address 0x%x for LD instruction\n",
          s_reg);
    }
    /* Load memory from BC into accumulator */
    z80_state.vcpu.acc = z80_state.ram[s_reg];
  /* LD A, (nn) */
  } else if (operand_1 == 0x3A) {
    operand_3 = z80_fetch_byte();
    operand_4 = z80_fetch_byte();
    t_reg = ((operand_4 << 8) | operand_3);

    /* Check if the loaded address is valid position in ram */
    if(!z80_ram_valid(t_reg)) {
      printf("Unknown target RAM address 0x%x for LD instruction\n",
          t_reg);
    }

    /* Load accumulator content into ram position */
    z80_state.ram[t_reg] = z80_state.vcpu.acc;

  /* LD (BC), A */
  } else if (operand_1 == 0x02) {
    t_reg = ((z80_state.vcpu.gp[reg_B] << 8) | z80_state.vcpu.gp[reg_C]);

    if(!z80_ram_valid(t_reg)) {
      printf("Unknown target RAM address 0x%x for LD (BC), A \
          instruction\n", t_reg);
    }
    z80_state.ram[t_reg] = z80_state.vcpu.acc;

  /* LD (DE), A */
  } else if (operand_1 == 0x12) {
    t_reg = ((z80_state.vcpu.gp[reg_D] << 8) | z80_state.vcpu.gp[reg_E]);

    if(!z80_ram_valid(t_reg)) {
      printf("Unknown target RAM address 0x%x for LD (DE), A \
          instruction\n", t_reg);
    }
    z80_state.ram[t_reg] = z80_state.vcpu.acc;

  /* LD (nn), A */
  } else if (operand_1 == 0x32) {
    operand_3 = z80_fetch_byte();
    operand_4 = z80_fetch_byte();
    s_reg = ((operand_4 << 8) | operand_3);

    /* Check if the loaded address is valid position in ram */
    if(!z80_ram_valid(s_reg)) {
      printf("Unknown target RAM address 0x%x for LD (nn), A \
          instruction\n", s_reg);
    }

    /* Load conent from ram position into accumulator */
    z80_state.vcpu.acc = z80_state.ram[s_reg];

  } else if (operand_1 == 0xED) {
    operand_2 = z80_fetch_byte();
    /* LD A, I */
    if (operand_2 == 0x57) {
      z80_state.vcpu.acc = z80_state.vcpu.i;
      z80_update_flags(z80_state.vcpu.i, SIGN_FLAG | ZERO_FLAG |
          HALFCARRY_FLAG | PARITYOVERFLOW_FLAG | ADDSUB_FLAG);
#ifdef DEBUG
      printf("LD, A, I\n");
#endif

    /* LD A, R */
    } else if (operand_2 == 0x5F) {
      z80_state.vcpu.acc = z80_state.vcpu.r;
      z80_update_flags(z80_state.vcpu.i, SIGN_FLAG | ZERO_FLAG |
          HALFCARRY_FLAG | PARITYOVERFLOW_FLAG | ADDSUB_FLAG);
#ifdef DEBUG
      printf("LD, A, R\n");
#endif

    /* LD I, A */
    } else if (operand_2 == 0x47) {
      z80_state.vcpu.i = z80_state.vcpu.acc;
#ifdef DEBUG
      printf("LD, I, A\n");
#endif

    /* LD R, A */
    } else if (operand_2 == 0x4F) {
      z80_state.vcpu.r = z80_state.vcpu.acc;
#ifdef DEBUG
      printf("LD, R, A\n");
#endif

    /* LD dd, (nn) */
    } else if ((operand_2 & 0xCF) == 0x4B) {
      operand_3 = z80_fetch_byte();
      operand_4 = z80_fetch_byte();
      /*Target: BC*/
      if((operand_2 & 0x30) == 0x00) {
        tmp = (operand_3 << 8 | operand_4);
        z80_state.vcpu.gp[reg_C] = z80_state.ram[tmp];
        z80_state.vcpu.gp[reg_B] = z80_state.ram[tmp + 1];

#ifdef DEBUG
        printf("LD BC, (0x%04x)\t; 0x%02x%02x\n", tmp, z80_state.vcpu.gp[reg_C], z80_state.vcpu.gp[reg_B]);
#endif
      /*Target: DE*/
      } else if((operand_2 & 0x30) == 0x10) {
        tmp = (operand_3 << 8 | operand_4);
        z80_state.vcpu.gp[reg_E] = z80_state.ram[tmp];
        z80_state.vcpu.gp[reg_D] = z80_state.ram[tmp + 1];
#ifdef DEBUG
        printf("LD DE, (0x%04x)\t; 0x%02x%02x\n", tmp, z80_state.vcpu.gp[reg_D], z80_state.vcpu.gp[reg_E]);
#endif
      /*Target: HL*/
      } else if((operand_2 & 0x30) == 0x20) {
        tmp = (operand_3 << 8 | operand_4);
        z80_state.vcpu.gp[reg_L] = z80_state.ram[tmp];
        z80_state.vcpu.gp[reg_H] = z80_state.ram[tmp + 1];
#ifdef DEBUG
        printf("LD HL, (0x%04x)\t; 0x%02x%02x\n", tmp, z80_state.vcpu.gp[reg_H], z80_state.vcpu.gp[reg_L]);
#endif
      /*Target: SP*/
      } else if((operand_2 & 0x30) == 0x30) {
        tmp = (operand_3 << 8 | operand_4);
        z80_state.vcpu.sp = (z80_state.ram[tmp + 1] << 8) | z80_state.ram[tmp];
#ifdef DEBUG
        printf("LD SP, (0x%04x)\t; 0x%04x\n", tmp, z80_state.vcpu.sp);
#endif
      }
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
      z80_state.vcpu.gp[reg_B] = operand_2;
      z80_state.vcpu.gp[reg_C] = operand_3;
#ifdef DEBUG
      printf("LD BC, 0x%02x%02x\n", operand_2, operand_3);
#endif
      break;
    case 1: /*DE*/
      z80_state.vcpu.gp[reg_D] = operand_2;
      z80_state.vcpu.gp[reg_E] = operand_3;
#ifdef DEBUG
      printf("LD DE, 0x%02x%02x\n", operand_2, operand_3);
#endif
      break;
    case 2: /*HL*/
      z80_state.vcpu.gp[reg_H] = operand_2;
      z80_state.vcpu.gp[reg_L] = operand_3;
#ifdef DEBUG
      printf("LD HL, 0x%02x%02x\n", operand_2, operand_3);
#endif
      break;
    case 3: /*SP*/
      z80_state.vcpu.sp  = (operand_3 << 8) | operand_2;
#ifdef DEBUG
      printf("LD SP, 0x%02x%02x\n", operand_2, operand_3);
#endif
      break;
    }

  /* LD HL, (nn) */
  } else if (operand_1 == 0x2A) {
    operand_3 = z80_fetch_byte();
    operand_4 = z80_fetch_byte();

    s_reg = ((operand_4 << 8) | operand_3);

    if(!z80_ram_valid(s_reg)) {
      printf("Unknown target RAM address 0x%x for LD HL, (nn) \
          instruction\n", s_reg);
    }
    z80_state.vcpu.gp[reg_L] = z80_state.ram[s_reg];

    /* Get nn+1 */
    s_reg++;

    /* Check if the loaded address is valid position in ram */
    if(!z80_ram_valid(s_reg)) {
      printf("Unknown target RAM address 0x%x for LD HL, (nn) \
          instruction\n", s_reg);
    }
    z80_state.vcpu.gp[reg_H] = z80_state.ram[s_reg];

  /* LD (nn), HL */
  } else if (operand_1 == 0x22) {
    operand_3 = z80_fetch_byte();
    operand_4 = z80_fetch_byte();

    t_reg = ((operand_4 << 8) | operand_3);

    if(!z80_ram_valid(t_reg)) {
      printf("Unknown target RAM address 0x%x for LD (nn), HL \
          instruction\n", t_reg);
    }
    z80_state.ram[t_reg] = z80_state.vcpu.gp[reg_L];

    /* Get nn+1 */
    t_reg++;

    /* Check if the loaded address is valid position in ram */
    if(!z80_ram_valid(t_reg)) {
      printf("Unknown target RAM address 0x%x for LD (nn), HL \
          instruction\n", t_reg);
    }
    z80_state.ram[t_reg] = z80_state.vcpu.gp[reg_H];

  /* LD SP, HL */
  } else if (operand_1 == 0xF9) {
    z80_state.vcpu.sp = (z80_state.vcpu.gp[reg_H] << 8) | z80_state.vcpu.gp[reg_L];

  /* PUSH qq */
  } else if ((operand_1 & 0xCF) == 0xC5) {
    s_reg = --z80_state.vcpu.sp;
    t_reg = --z80_state.vcpu.sp;

    if(!z80_ram_valid(s_reg)) {
      printf("Unknown target RAM address 0x%x for PUSH qq \
          instruction\n", s_reg);
    }

    if(!z80_ram_valid(t_reg)) {
      printf("Unknown target RAM address 0x%x for PUSH qq \
          instruction\n", t_reg);
    }

    switch((operand_1 & 0x30) >> 4) {
    case 0: /*BC*/
      z80_state.ram[s_reg] = z80_state.vcpu.gp[reg_B];
      z80_state.ram[t_reg] = z80_state.vcpu.gp[reg_C];
      break;
    case 1: /*DE*/
      z80_state.ram[s_reg] = z80_state.vcpu.gp[reg_D];
      z80_state.ram[t_reg] = z80_state.vcpu.gp[reg_E];
      break;
    case 2: /*HL*/
      z80_state.ram[s_reg] = z80_state.vcpu.gp[reg_H];
      z80_state.ram[t_reg] = z80_state.vcpu.gp[reg_L];
      break;
    case 3: /*AF*/
      z80_state.ram[s_reg] = z80_state.vcpu.acc;
      z80_state.ram[t_reg] = z80_state.vcpu.flags;
      break;
    }

  /* POP qq */
  } else if ((operand_1 & 0xCF) == 0xC1) {
    s_reg = z80_state.vcpu.sp++;
    t_reg = z80_state.vcpu.sp++;

    if(!z80_ram_valid(s_reg)) {
      printf("Unknown target RAM address 0x%x for PUSH qq \
          instruction\n", s_reg);
    }

    if(!z80_ram_valid(t_reg)) {
      printf("Unknown target RAM address 0x%x for PUSH qq \
          instruction\n", t_reg);
    }

    switch((operand_1 & 0x30) >> 4) {
    case 0: /*BC*/
      z80_state.vcpu.gp[reg_B] = z80_state.ram[s_reg];
      z80_state.vcpu.gp[reg_C] = z80_state.ram[t_reg];
      break;
    case 1: /*DE*/
      z80_state.vcpu.gp[reg_D] = z80_state.ram[s_reg];
      z80_state.vcpu.gp[reg_E] = z80_state.ram[t_reg];
      break;
    case 2: /*HL*/
      z80_state.vcpu.gp[reg_H] = z80_state.ram[s_reg];
      z80_state.vcpu.gp[reg_L] = z80_state.ram[t_reg];
      break;
    case 3: /*AF*/
      z80_state.vcpu.acc = z80_state.ram[s_reg];
      z80_state.vcpu.flags = z80_state.ram[t_reg];
      break;
    }

  /* Block Transfer and Search */
  /* EX DE, HL */
  } else if(operand_1 == 0xEB) {

    z80_swap_reg(&z80_state.vcpu.gp[reg_D], &z80_state.vcpu.gp[reg_H]);
    z80_swap_reg(&z80_state.vcpu.gp[reg_E], &z80_state.vcpu.gp[reg_L]);

  /* EX AF, AF' */
  } else if(operand_1 == 0x08) {
    z80_swap_reg(&z80_state.vcpu.acc, &z80_state.vcpu.acc_);
    z80_swap_reg(&z80_state.vcpu.flags, &z80_state.vcpu.flags_);

  /* EXX */
  } else if(operand_1 == 0xD9) {
    z80_swap_reg(&z80_state.vcpu.gp[reg_B], &z80_state.vcpu.gp[reg_B_]);
    z80_swap_reg(&z80_state.vcpu.gp[reg_C], &z80_state.vcpu.gp[reg_C_]);
    z80_swap_reg(&z80_state.vcpu.gp[reg_D], &z80_state.vcpu.gp[reg_D_]);
    z80_swap_reg(&z80_state.vcpu.gp[reg_E], &z80_state.vcpu.gp[reg_E_]);
    z80_swap_reg(&z80_state.vcpu.gp[reg_H], &z80_state.vcpu.gp[reg_H_]);
    z80_swap_reg(&z80_state.vcpu.gp[reg_L], &z80_state.vcpu.gp[reg_L_]);

  /* EX (SP), HL */
  } else if(operand_1 == 0xE3) {
    s_reg = z80_state.vcpu.sp;
    t_reg = z80_state.vcpu.sp+1;

    if(!z80_ram_valid(s_reg)) {
      printf("Unknown target RAM address 0x%x for PUSH qq \
          instruction\n", s_reg);
    }

    if(!z80_ram_valid(t_reg)) {
      printf("Unknown target RAM address 0x%x for PUSH qq \
          instruction\n", t_reg);
    }

    z80_swap_reg(&z80_state.ram[s_reg], &z80_state.vcpu.gp[reg_L]);
    z80_swap_reg(&z80_state.ram[t_reg], &z80_state.vcpu.gp[reg_H]);

//  /* EX (SP), IY */
//  } else if(1) {

  /* Arithmetic and Logical */
  /* ADD A, r */
  } else if((operand_1 & 0x07) == 0x80) {
    /* TODO: Set condition flags correctly */
    s_reg = z80_get_s_reg(operand_1);
    z80_state.vcpu.acc += s_reg;

  /* ADD A, n */
  } else if(operand_1 == 0xC6) {
    /* TODO: Set condition flags correctly */
    operand_2 = z80_fetch_byte();
    z80_state.vcpu.acc += operand_2;

  /* ADD A, (HL) */
  } else if(operand_1 == 0x86) {
    /* TODO: Set condition flags correctly */
    s_reg = (z80_state.vcpu.gp[reg_H] << 8) | z80_state.vcpu.gp[reg_L];

    if(!z80_ram_valid(s_reg)) {
      printf("Unknown target RAM address 0x%x for PUSH qq \
          instruction\n", s_reg);
    }
    z80_state.vcpu.acc += z80_state.ram[s_reg];

  /* ADD A, (IY+d) */
//  } else if(1) {
//  /* ADC A, s */
//  } else if(1) {
  /* SUB r */
  } else if((operand_1 & 0xF8) ==  0x90) {
    /* TODO: Set condition flags correctly */
    s_reg = z80_get_s_reg(operand_1);
    z80_state.vcpu.acc -= s_reg;

  /* SUB n */
  } else if (operand_1 == 0xD6) {
    /* TODO: Set condition flags correctly */
    operand_2 = z80_fetch_byte();
    z80_state.vcpu.acc -= operand_2;

//  /* SBC A, s */
//  } else if(1) {
//  /* AND s */
//  } else if(1) {
//  /* OR s */
//  } else if(1) {
//  /* XOR s */
//  } else if(1) {
//  /* CP s*/
//  } else if(1) {
//  /* INC r */
//  } else if(1) {
//  /* INC (HL) */
//  } else if(1) {
//  /* INC (IX+d) */
//  } else if(1) {
//  /* INC (IY+d) */
//  } else if(1) {
//  /* DEC m */
//  } else if(1) {
//  /* DAA */
//  } else if(1) {
//  /* CPL */
//  } else if(1) {
//  /* NEG */
//  } else if(1) {
//  /* CCF */
//  } else if(1) {
//  /* SCF */
//  } else if(1) {
//  /* NOP */
//  } else if(1) {
//  /* HALT */
//  } else if(1) {
  /* DI */
  } else if(operand_1 == 0xF3) {
    z80_state.vcpu.iff1 = 0x0;
    z80_state.vcpu.iff2 = 0x0;

#ifdef DEBUG
    printf("DI\n");
#endif

  /* EI */
  } else if(operand_1 == 0xFB) {
    z80_state.vcpu.iff1 = 0x1;
    z80_state.vcpu.iff2 = 0x1;
#ifdef DEBUG
    printf("EI\n");
#endif
//  /* IM 0 */
//  } else if(1) {
//  /* IM 1 */
//  } else if(1) {
//  /* IM 2 */
//
//
//  /* ADD HL, ss */
//  } else if(1) {
//  /* ADC HL, ss */
//  } else if(1) {
//  /* SBC HL, ss */
//  } else if(1) {
//  /* ADD IX, pp */
//  } else if(1) {
//  /* ADD IY, rr */
//  } else if(1) {
//  /* INC ss */
//  } else if(1) {
//  /* INC IX */
//  } else if(1) {
//  /* INC IY */
//  } else if(1) {
//  /* DEC ss */
//  } else if(1) {
//  /* DEC IX */
//  } else if(1) {
//  /* DEC IY */
//  } else if(1) {
//
//  /* Rotate and Shift */
//  /* RLCA */
//  } else if(1) {
//  /* RLA */
//  } else if(1) {
//  /* RLCA */
//  } else if(1) {
//  /* RRA */
//  } else if(1) {
  /* RLC r */
  } else if(operand_1 == 0xCB) {
    /* TODO: Set condition flags correctly */
    s_reg = z80_get_s_reg(z80_fetch_byte());
    z80_state.vcpu.gp[s_reg] <<= 1;
#ifdef DEBUG
    printf("RLC %s\t; 0x%02x\n", z80_decode_gp_reg(s_reg), s_reg);
#endif

//  /* RLC (HL) */
//  } else if(1) {
//  /* RLC (IX+d) */
//  } else if(1) {
//  /* RLC (IY+d) */
//  } else if(1) {
//  /* RL m */
//  } else if(1) {
//  /* RRC m */
//  } else if(1) {
//  /* RR m */
//  } else if(1) {
//  /* SLA m */
//  } else if(1) {
//  /* SRA m */
//  } else if(1) {
//  /* SRL m */
//  } else if(1) {
//  /* RLD */
//  } else if(1) {
//  /* RRD */
//  } else if(1) {
//
//  /* Bit Manipulation (Set, Reset, Test) */
//  /* BIT b, r */
//  } else if(1) {
//  /* BIT b, (HL) */
//  } else if(1) {
//  /* BIT b, (IX+d) */
//  } else if(1) {
//  /* BIT b, (IY+d) */
//  } else if(1) {
//  /* SET b, r */
//  } else if(1) {
//  /* SET b, (HL) */
//  } else if(1) {
//  /* SET b, (IX+d) */
//  } else if(1) {
//  /* SET b, (IY+d) */
//  } else if(1) {
//  /* RES b, m */
//  } else if(1) {

  /* Jump, Call, and Return */
  /* JP nn */
  } else if(operand_1 == 0xC3) {
    operand_2 = z80_fetch_byte();
    operand_3 = z80_fetch_byte();

    s_reg = ((operand_3 << 8) | operand_2);

    if(!z80_ram_valid(s_reg)) {
      printf("Unknown target RAM address 0x%x for JP nn instruction\n", s_reg);
    }
    z80_state.vcpu.pc = z80_state.ram[s_reg];

  /* JP cc, nn */
  } else if((operand_1 & 0xC7) == 0xC2) {
    operand_2 = z80_fetch_byte();
    operand_3 = z80_fetch_byte();

    if(z80_condition_true(operand_1 & 0x38 >> 3)) {
      s_reg = ((operand_3 << 8) | operand_2);
      z80_state.vcpu.pc = s_reg;
#ifdef DEBUG
      printf("JP cc, 0x%04x\t; true\n", s_reg);
#endif
    } else {
#ifdef DEBUG
      printf("JP cc, 0x%04x\t; false\n", s_reg);
#endif
    }
  /* JR e */
  } else if(operand_1 == 0x18) {
    operand_2 = z80_fetch_byte();
    s_reg = z80_state.vcpu.pc + operand_2;
    if(!z80_ram_valid(s_reg)) {
      printf("Unknown target RAM address 0x%x for JR e \
          instruction\n", s_reg);
    }
    z80_state.vcpu.pc = s_reg;

  /* JR C, e */
  } else if(operand_1 == 0x38) {
    if(z80_flag_set(z80_state.vcpu.flags, CARRY_FLAG)) {
      operand_2 = z80_fetch_byte();
      if(!z80_ram_valid(z80_state.vcpu.pc + operand_2)) {
        printf("Unknown target RAM address 0x%x for JR C, e \
            instruction\n", s_reg);
      }
      /*TODO: Check if this is right not sure if I have to add 1*/
      z80_state.vcpu.pc += operand_2;
    } else {
      /* Just skip the branch value*/
      z80_fetch_byte();
    }

  /* JR NC, e */
  } else if(operand_1 == 0x30) {
    if(!z80_flag_set(z80_state.vcpu.flags, CARRY_FLAG)) {
      operand_2 = z80_fetch_byte();
      if(!z80_ram_valid(z80_state.vcpu.pc + operand_2)) {
        printf("Unknown target RAM address 0x%x for JR C, e \
            instruction\n", s_reg);
      }
      /*TODO: Check if this is right not sure if I have to add 1*/
      z80_state.vcpu.pc += operand_2;
    } else {
      /* Just skip the branch value*/
      z80_fetch_byte();
    }

  /* JR Z, e */
  } else if(operand_1 == 0x28) {
    operand_2 = z80_fetch_byte();
    if(z80_flag_set(z80_state.vcpu.flags, ZERO_FLAG)) {
      if(!z80_ram_valid(z80_state.vcpu.pc + operand_2)) {
        printf("Unknown target RAM address 0x%x for JR Z, e instruction\n", s_reg);
      }
      z80_state.vcpu.pc += operand_2;
#ifdef DEBUG
    printf("JR Z, 0x%04x\t; true\n", z80_state.vcpu.pc);
#endif
    } else {
#ifdef DEBUG
      printf("JR Z, 0x%04x\t; false\n", z80_state.vcpu.pc);
#endif
    }

  /* JR NZ, e */
  } else if(operand_1 == 0x20) {
    if(!z80_flag_set(z80_state.vcpu.flags, ZERO_FLAG)) {
      operand_2 = z80_fetch_byte();
      if(!z80_ram_valid(z80_state.vcpu.pc + operand_2)) {
        printf("Unknown target RAM address 0x%x for JR Z, e \
            instruction\n", s_reg);
      }
      /*TODO: Check if this is right not sure if I have to add 1*/
      z80_state.vcpu.pc += operand_2;
    } else {
      /* Just skip the branch value*/
      z80_fetch_byte();
    }

  /* JP (HL) */
  } else if(operand_1 == 0xE9) {
#ifdef DEBUG
    printf("jp (HL)\n");
#endif
    s_reg = ((z80_state.vcpu.gp[reg_H] << 8) | z80_state.vcpu.gp[reg_L]);

    if (!z80_ram_valid(s_reg)) {
      printf("Unkown source RAM address %u for JP (HL) instruction\n",
          s_reg);
    }
    z80_state.vcpu.pc = z80_state.ram[s_reg];


//  /* JP (IX) */
//  } else if(1) {
//
//  /* JP (IY) */
//  } else if(1) {
//  /* DJNZ, e */
//  } else if(1) {
  /* CALL nn */
  } else if(operand_1 == 0xCD) {
    tmp = (z80_fetch_byte()) | (z80_fetch_byte() << 8);
    z80_state.vcpu.sp--;
    z80_state.stack[z80_state.vcpu.sp--] = (uint8_t)((z80_state.vcpu.pc & 0xff00) >> 8);
    z80_state.stack[z80_state.vcpu.sp] = (uint8_t)(z80_state.vcpu.pc & 0x00ff);
    z80_state.vcpu.pc = tmp;
#ifdef DEBUG
    printf("CALL 0x%0x (PC=0x%04x, SP=0x%04x)\n", z80_state.vcpu.pc, z80_state.vcpu.sp);
    dump_stack();
#endif

//  /* CALL cc, nn */
//  } else if(1) {
  /* RET */
  } else if(operand_1 == 0xC9) {
    z80_state.vcpu.pc = z80_state.stack[z80_state.vcpu.sp];
    z80_state.vcpu.sp++;

#ifdef DEBUG
    printf("RET\n");
#endif
  /* RET cc */
  } else if((operand_1 & 0xC7) == 0xC0) {
    tmp = z80_state.stack[z80_state.vcpu.sp++] | (z80_state.stack[z80_state.vcpu.sp] << 8);
    z80_state.vcpu.sp++;
    z80_state.vcpu.pc = tmp;
#ifdef DEBUG
    printf("RET cc ; 0x%04x\n", z80_state.vcpu.pc);

    dump_stack();
#endif
//  /* RETI */
//  } else if(1) {
//  /* RETN */
//  } else if(1) {
//  /*RST p */
//  } else if(1) {
//
//  /* Input/Output */
  /* IN A, (n) */
  } else if(operand_1 == 0xdb) {
    z80_state.vcpu.acc = z80_fetch_byte();
#ifdef DEBUG
    printf("IN A, (0x%02x)\n", z80_state.vcpu.acc);
#endif
  }
//  /* IN r (C)*/
//  } else if(1) {
//  /* INI */
//  } else if(1) {
//  /* INIR */
//  } else if(1) {
//  /* IND */
//  } else if(1) {
//  /* INDR */
//  } else if(1) {
//  /* OUT (n), A */
//  } else if(1) {
//  /* OUT (C), r */
//  } else if(1) {
//  /* OUTI */
//  } else if(1) {
//  /* OTIR */
//  } else if(1) {
//  /* OUTD */
//  } else if(1) {
//  /* OTDR */
//  } else if(1) {
//  }

  else {
    printf("Decode missing for 0x%02x @0x%0x\n", operand_1, z80_state.vcpu.pc);
    return;
  }
}

