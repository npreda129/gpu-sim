#ifndef LANE_H
#define LANE_H

#include <stdint.h>

#define REGS_INT_PER_LANE 8 
#define REGS_FLOAT_PER_LANE 6
#define REGS_SPECIAL_PER_LANE 2
#define REGS_PER_LANE REGS_INT_PER_LANE + REGS_FLOAT_PER_LANE + REGS_SPECIAL_PER_LANE
#define REG_FLAGS 15
#define REG_THREAD_ID 14
// the format of thread id is tbd, but we will cast it as a struct
// later to parse it

#define FLAG_MASK_ZERO 0x1
#define FLAG_MASK_SIGN 0x2

typedef struct lane {
  uint32_t regs_int[REGS_INT_PER_LANE];
  float regs_float[REGS_FLOAT_PER_LANE];
  uint32_t rThread;
  uint32_t rFlags;
} lane_t;

void * register_access(lane_t *lane, int regNum);

#endif
