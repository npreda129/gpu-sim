#define REGS_PER_LANE 8

typedef struct lane {
  uint32_t gpRegs[REGS_PER_LANE];
  uint8_t flags;
} lane_t;
