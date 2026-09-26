#include "core.h"

void core_init(core_t *core, int numLanes) {
  *core = malloc(sizeof(core_t));
  core->numLanes = numLanes;
  for (int i = 0; i < numLanes; i++) {
    core->lanes[i] = malloc(sizeof(lane_t));
  }
}

void core_step(core_t *core) {
  uint32_t instruction = core->binaryMem[core->pc]
  lane_t *current_lane;
  for (int i = 0; i < core->numLanes; i++) {
    current_lane = core->lanes[i];
    // process instruction

  }
}
