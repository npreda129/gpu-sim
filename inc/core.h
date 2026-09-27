#ifndef CORE_H
#define CORE_H

#include "lane.h"
#include "instruction.h"
#include <stdlib.h>
#include <stdint.h>


#define MAX_B_SIZE 1024 // 1k instructions, 4kb
#define S_MEM_SIZE 4194304 // 4mb

// counted per lane, except instructions, which is how many times the shared
// instruction pointer issued one
typedef struct core_stats {
  uint64_t instructions;
  uint64_t laneOps;
  uint64_t loads;
  uint64_t stores;
  uint64_t flops;
} core_stats_t;

typedef struct core {
  int rip;
  // later, add intermediate structures
  int numLanes;
  lane_t ** lanes;

  core_stats_t stats;
  int binarySize; // bytes of binaryMem holding the current kernel
  int tracing; // print each instruction of this launch (trace level 2+)
  
  uint32_t binaryMem[MAX_B_SIZE];
  uint8_t sharedMem[S_MEM_SIZE];
} core_t;

void core_init(core_t *core, int numLanes);
int core_step(core_t *core);
void core_execute_r(core_t *core, lane_t *lane, uint32_t * regs[3], int opcode, int isFloat);

#endif
