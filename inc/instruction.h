#ifndef INSTRUCTION_H
#define INSTRUCTION_H

// Shifts (applied first)
#define SHIFT_OP 28u
#define SHIFT_RS 24u
#define SHIFT_RT 20u
#define SHIFT_RD 16u
#define SHIFT_IMM 0

// Masks (applied second)
#define MASK_OP 0xF
#define MASK_REG 0xF
#define MASK_IMM 0xFFFFFF

// Opcodes
#define OP_FMA 0
#define OP_LW 1u
#define OP_SW 2u
#define OP_CMP 3u
#define OP_CMOV 4u
#define OP_HALT 5u
#define OP_SHL 6u
#define OP_SHR 7u
#define OP_LI 8u
#define OP_JUMP 9u

#endif
