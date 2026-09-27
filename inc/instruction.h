#ifndef INSTRUCTION_H
#define INSTRUCTION_H

// Shifts (applied first)
#define SHIFT_OP 28
#define SHIFT_RS 24
#define SHIFT_RT 20
#define SHIFT_RD 16
#define SHIFT_IMM 0

// Masks (applied second)
#define MASK_OP 0xF
#define MASK_REG 0xF
#define MASK_IMM 0xFFFFFF

// Opcodes
#define OP_FMA 0
#define OP_LW 1
#define OP_SW 2
#define OP_CMP 3
#define OP_CMOV 4
#define OP_LI 8
#define OP_HALT 5
#define OP_JUMP 9

#endif
