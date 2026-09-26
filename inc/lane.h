#define REGS_PER_LANE 8

typedef struct 
typedef struct lane {
  uint32_t gpRegs[REGS_PER_LANE];
  uint8_t flags;
  thread_addr_t threadID;
} lane_t;
