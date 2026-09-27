#include "lane.h"
#include <stddef.h>

void * register_access(lane_t *lane, int regNum) {
  if (regNum < 8) {
    return lane->regs_int + regNum;
  }

  if (regNum < 14) {
    return lane->regs_float + (regNum - 8);
  }

  if (regNum == 14) {
    return &(lane->rThread);
  }

  if (regNum == 15) {
    return &(lane->rFlags);
  }

  return NULL;
}
