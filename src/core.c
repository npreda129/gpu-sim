#include "core.h"

void core_init(core_t *core, int numLanes) {
  *core = malloc(sizeof(core_t));
  core->numLanes = numLanes;

  for (int i = 0; i < numLanes; i++) {
    core->lanes[i] = malloc(sizeof(lane_t));
  }
}

void core_step(core_t *core) { //this will later be renamed block_step
  lane_t *currentLane;
  uint32_t * regs[3];
  int opcode, imm;
  uint32_t instruction = core->binaryMem[core->rip];

  opcode = (instruction >> SHIFT_OP) & MASK_OP;
  if (opcode == OP_HALT) {
    return;
  }
  if (opcode & 0x8) {
    // prepare and execute I format
    imm = (instruction >> SHIFT_IMM) & MASK_IMM;
    for (int i = 0; i < core->numLanes; i++) {
      // set register pointers for readability
      currentLane = core->lanes[i];
      regs[0] = &(currentLane->regs[(instruction >> SHIFT_RS) & MASK_REG]);
      // core_execute_i(regs, imm);
      *regs[0] = imm;
    }
  } else {
    // prepare and execute R format
    for (int i = 0; i < core->numLanes; i++) {
      // set register pointers for readability
      currentLane = core->lanes[i];
      regs[0] = &(currentLane->regs[(instruction >> SHIFT_RS) & MASK_REG]);
      regs[1] = &(currentLane->regs[(instruction >> SHIFT_RT) & MASK_REG]);
      regs[2] = &(currentLane->regs[(instruction >> SHIFT_RD) & MASK_REG]);
      core_execute_r(core, currentLane, regs, opcode);
    }
  }

  core->rip++;
}

void core_execute_r(core_t *core, lane_t *lane, uint32_t * regs[3], int opcode) {
  uint32_t *rS = regs[0];
  uint32_t *rT = regs[1];
  uint32_t *rD = regs[2];
  uint32_t flags = &(lane->regs[REG_FLAGS]);
  switch (opcode) {
    case OP_FMA:
      *rD += *rS * *rT;
      break;
    case OP_LW:
      *rD = core->sharedMem[*rS];
      break;
    case OP_SW:
      core->sharedMem[*rD] = *rS;
      break;
    case OP_CMP:
      // if rS - rT is positive, unset zero flag. otherwise set it
      *flags = (*rS > *rT ? *flags & !FLAG_MASK_ZERO : *flags | FLAG_MASK_ZERO)
      break;
    case OP_CMOV:
      // if zero flag set, rD := rS. else rD := rT
      *rD = (*flags & FLAG_MASK_ZERO ? *rS : *rT)
      break;
  }
}

// void core_execute_i(uint32_t * regs[3], int imm) {
//   //opcode is not a parameter because there is only one I-format instruction now
//   *regs[0] = imm;
// }


