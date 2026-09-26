#include "lane.h"

#define MAX_B_SIZE 1024 // 1k instructions, 4kb
#define S_MEM_SIZE 1024 // 1kb

typedef struct core {
  int pc;
  // later, add intermediate structures
  int numLanes;
  lane_t ** lanes;
  
  uint32_t binaryMem[MAX_B_SIZE];
  uint8_t sharedMem[S_MEM_SIZE];
} core_t;

void core_init(core_t *core);
void core_step(core_t *core);

