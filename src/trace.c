#include "trace.h"
#include "instruction.h"
#include "lane.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int trace_level = 1;
int trace_lanes = 2;
int trace_launches = 1;

static const char *opNames[16] = {
  "FMA", "LW", "SW", "CMP", "CMOV", "HALT", "SHL", "SHR", "LI", "JUMP",
  "OP10", "OP11", "OP12", "OP13", "OP14", "OP15"
};

static const char *regNames[16] = {
  "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
  "r8", "r9", "r10", "r11", "r12", "r13", "rT", "rF"
};

static int env_int(const char *name, int fallback) {
  char *value = getenv(name);
  return value ? atoi(value) : fallback;
}

void trace_init(void) {
  trace_level = env_int("GPU_TRACE", trace_level);
  trace_lanes = env_int("GPU_TRACE_LANES", trace_lanes);
  trace_launches = env_int("GPU_TRACE_LAUNCHES", trace_launches);
  // flush every line, so the trace keeps up when piped (e.g. into tee)
  setvbuf(stdout, NULL, _IOLBF, 0);
}

const char *trace_reg_name(int regNum) {
  return regNames[regNum & MASK_REG];
}

void trace_disassemble(uint32_t instruction, char *buf, size_t len) {
  int opcode = (instruction >> SHIFT_OP) & MASK_OP;
  const char *rs = regNames[(instruction >> SHIFT_RS) & MASK_REG];
  const char *rt = regNames[(instruction >> SHIFT_RT) & MASK_REG];
  const char *rd = regNames[(instruction >> SHIFT_RD) & MASK_REG];

  if (opcode == OP_HALT) {
    snprintf(buf, len, "HALT");
  } else if (opcode & 0x8) {
    snprintf(buf, len, "%s %s %u", opNames[opcode], rs,
        (instruction >> SHIFT_IMM) & MASK_IMM);
  } else {
    snprintf(buf, len, "%s %s %s %s", opNames[opcode], rs, rt, rd);
  }
}

void trace_explain(uint32_t instruction, char *buf, size_t len) {
  int opcode = (instruction >> SHIFT_OP) & MASK_OP;
  int rdNum = (instruction >> SHIFT_RD) & MASK_REG;
  const char *rs = regNames[(instruction >> SHIFT_RS) & MASK_REG];
  const char *rt = regNames[(instruction >> SHIFT_RT) & MASK_REG];
  const char *rd = regNames[rdNum];
  unsigned imm = (instruction >> SHIFT_IMM) & MASK_IMM;

  switch (opcode) {
    case OP_FMA:
      snprintf(buf, len, "%s += %s * %s (%s multiply-add)", rd, rs, rt,
          register_is_float(rdNum) ? "float" : "int");
      break;
    case OP_LW:
      snprintf(buf, len, "%s = mem[%s] (load from shared memory)", rd, rs);
      break;
    case OP_SW:
      snprintf(buf, len, "mem[%s] = %s (store to shared memory)", rd, rs);
      break;
    case OP_CMP:
      snprintf(buf, len, "rF = compare %s, %s (Z if equal, S if %s <= %s)", rs, rt, rs, rt);
      break;
    case OP_CMOV:
      snprintf(buf, len, "%s = Z ? %s : %s (select instead of branching)", rd, rs, rt);
      break;
    case OP_HALT:
      snprintf(buf, len, "kernel finished");
      break;
    case OP_SHL:
      snprintf(buf, len, "%s = %s << %s", rd, rs, rt);
      break;
    case OP_SHR:
      snprintf(buf, len, "%s = %s >> %s", rd, rs, rt);
      break;
    case OP_LI:
      snprintf(buf, len, "%s = %u (load immediate)", rs, imm);
      break;
    default:
      // core_step treats every opcode with the high bit set like LI
      snprintf(buf, len, "%s = %u (not implemented, runs as LI)", rs, imm);
  }
}

void trace_format_value(uint32_t bits, int isFloat, char *buf, size_t len) {
  if (isFloat) {
    float f;
    memcpy(&f, &bits, 4);
    snprintf(buf, len, "%g", f);
  } else {
    snprintf(buf, len, "0x%08x (%u)", bits, bits);
  }
}
