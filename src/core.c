#include "core.h"
#include "trace.h"
#include <stdio.h>
#include <string.h>

void core_init(core_t *core, int numLanes) {
  // *core = malloc(sizeof(core_t));
  core->lanes = malloc(sizeof(lane_t *) * numLanes);
  core->numLanes = numLanes;
  core->rip = 0;
  core->binarySize = 0;
  core->tracing = 0;
  memset(&core->stats, 0, sizeof(core_stats_t));

  for (int i = 0; i < numLanes; i++) {
    core->lanes[i] = malloc(sizeof(lane_t));
  }

  trace_init();
  if (trace_level >= 1) {
    printf("[gpu] core_init: %d lanes share ONE instruction pointer (SIMT).\n", numLanes);
    printf("[gpu]   every instruction runs in all %d lanes at once, each on its own registers:\n",
        numLanes);
    printf("[gpu]   r0-r7 int, r8-r13 float, rT thread id, rF flags. %d KB of shared memory.\n",
        S_MEM_SIZE / 1024);
    printf("[gpu]   there are no branches: lanes that disagree use CMP + CMOV to pick a result.\n");
    printf("[gpu]   GPU_TRACE=0..3 sets how much is printed (now %d).\n", trace_level);
  }
}

// level 3 follows trace_lanes lanes spread evenly across the core
static int is_sample_lane(core_t *core, int lane) {
  int step = trace_lanes > 0 ? core->numLanes / trace_lanes : 0;
  if (step < 1) {
    step = 1;
  }
  return trace_lanes > 0 && lane % step == 0 && lane / step < trace_lanes;
}

// where the instruction leaves its result in this lane, so level 3 can show
// it before and after. returns NULL if there is nothing to show
static uint8_t *result_location(core_t *core, lane_t *lane, uint32_t instruction,
    int *isFloat, char *name, size_t len) {
  int opcode = (instruction >> SHIFT_OP) & MASK_OP;
  int rs = (instruction >> SHIFT_RS) & MASK_REG;
  int rd = (instruction >> SHIFT_RD) & MASK_REG;
  uint32_t addr;

  if (opcode & 0x8) {
    // immediates are raw bits, even in float registers
    *isFloat = 0;
    snprintf(name, len, "%s", trace_reg_name(rs));
    return register_access(lane, rs);
  }
  switch (opcode) {
    case OP_SW:
      addr = *(uint32_t *)register_access(lane, rd);
      if (addr > S_MEM_SIZE - 4) {
        return NULL;
      }
      *isFloat = register_is_float(rs);
      snprintf(name, len, "mem[0x%x]", addr);
      return core->sharedMem + addr;
    case OP_LW:
      addr = *(uint32_t *)register_access(lane, rs);
      *isFloat = register_is_float(rd);
      snprintf(name, len, "%s <- mem[0x%x]", trace_reg_name(rd), addr);
      return register_access(lane, rd);
    case OP_CMP:
      *isFloat = 0;
      snprintf(name, len, "rF");
      return register_access(lane, REG_FLAGS);
    default:
      *isFloat = register_is_float(rd);
      snprintf(name, len, "%s", trace_reg_name(rd));
      return register_access(lane, rd);
  }
}

static void execute_lane(core_t *core, lane_t *lane, uint32_t instruction) {
  int opcode = (instruction >> SHIFT_OP) & MASK_OP;
  uint32_t * regs[3];

  if (opcode & 0x8) {
    // I format
    regs[0] = register_access(lane, (instruction >> SHIFT_RS) & MASK_REG);
    // core_execute_i(regs, imm);
    *regs[0] = (instruction >> SHIFT_IMM) & MASK_IMM;
  } else {
    // R format
    regs[0] = register_access(lane, (instruction >> SHIFT_RS) & MASK_REG);
    regs[1] = register_access(lane, (instruction >> SHIFT_RT) & MASK_REG);
    regs[2] = register_access(lane, (instruction >> SHIFT_RD) & MASK_REG);
    core_execute_r(core, lane, regs, opcode,
        register_is_float((instruction >> SHIFT_RD) & MASK_REG));
  }
}

int core_step(core_t *core) { //this will later be renamed block_step
  uint32_t instruction = core->binaryMem[core->rip];
  int opcode = (instruction >> SHIFT_OP) & MASK_OP;
  char text[64], meaning[96], name[32], before[32], after[32];

  if (core->tracing) {
    trace_disassemble(instruction, text, sizeof(text));
    trace_explain(instruction, meaning, sizeof(meaning));
    printf("[gpu]   pc %3d  %-16s ; %s\n", core->rip, text, meaning);
  }

  if (opcode == OP_HALT) {
    // 0 will indicate the kernel is done
    return 0;
  }
  core->stats.instructions++;
  core->stats.laneOps += core->numLanes;

  for (int i = 0; i < core->numLanes; i++) {
    lane_t *lane = core->lanes[i];
    uint8_t *result = NULL;
    uint32_t old, new;
    int isFloat;

    if (core->tracing && trace_level >= 3 && is_sample_lane(core, i)) {
      result = result_location(core, lane, instruction, &isFloat, name, sizeof(name));
    }
    if (result) {
      memcpy(&old, result, 4);
    }

    execute_lane(core, lane, instruction);

    if (result) {
      memcpy(&new, result, 4);
      trace_format_value(old, isFloat, before, sizeof(before));
      trace_format_value(new, isFloat, after, sizeof(after));
      printf("[gpu]           lane %-5d %s: %s -> %s\n", i, name, before, after);
    }
  }

  core->rip++;
  // 1 will indicate there are still instructions to be executed
  return 1;
}

void core_execute_r(core_t *core, lane_t *lane, uint32_t * regs[3], int opcode, int isFloat) {
  uint32_t *rS = regs[0];
  uint32_t *rT = regs[1];
  uint32_t *rD = regs[2];
  uint32_t *flags = register_access(lane, REG_FLAGS);
  switch (opcode) {
    case OP_FMA:
      // a float destination register means the operands are treated as floats
      if (isFloat) {
        *(float *)rD += *(float *)rS * *(float *)rT;
        core->stats.flops += 2;
      } else {
        *rD += *rS * *rT;
      }
      break;
    case OP_LW:
      core->stats.loads++;
      memcpy(rD, &(core->sharedMem[*rS]), 4);
      break;
    case OP_SW:
      core->stats.stores++;
      memcpy(&(core->sharedMem[*rD]), rS, 4);
      break;
    case OP_CMP:
      // if rS - rT is positive, unset sign flag. otherwise set it
      *flags = (*rS > *rT ? *flags & ~FLAG_MASK_SIGN : *flags | FLAG_MASK_SIGN);
      // if rS == rT, set the zero flag. otherwise unset it
      *flags = (*rS == *rT ? *flags | FLAG_MASK_ZERO : *flags & ~FLAG_MASK_ZERO);
      break;
    case OP_CMOV:
      // if zero flag set, rD := rS. else rD := rT
      *rD = (*flags & FLAG_MASK_ZERO ? *rS : *rT);
      break;
    case OP_SHL:
      *rD = *rS << *rT;
      break;
    case OP_SHR:
      *rD = *rS >> *rT;
      break;
  }
}

// void core_execute_i(uint32_t * regs[3], int imm) {
//   //opcode is not a parameter because there is only one I-format instruction now
//   *regs[0] = imm;
// }


