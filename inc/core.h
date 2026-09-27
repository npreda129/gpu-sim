#ifndef CORE_H
#define CORE_H

#include "lane.h"
#include "instruction.h"
#include <stdlib.h>
#include <stdint.h>


#define MAX_B_SIZE 1024 // 1k instructions, 4kb
#define S_MEM_SIZE 1024 // 1kb

typedef struct core {
  int rip;
  // later, add intermediate structures
  int numLanes;
  lane_t ** lanes;
  
  uint32_t binaryMem[MAX_B_SIZE];
  uint8_t sharedMem[S_MEM_SIZE];
} core_t;

void core_init(core_t *core, int numLanes);
void core_step(core_t *core);
void core_execute_r(core_t *core, lane_t *lane, uint32_t * regs[3], int opcode);

#endif
