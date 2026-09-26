#define REGS_PER_LANE 16 
#define REG_FLAGS 15
#define REG_THREAD_ID 14
// the format of thread id is tbd, but we will cast it as a struct
// later to parse it

#define FLAG_MASK_ZERO 0x1

typedef struct lane {
  uint32_t regs[REGS_PER_LANE];
} lane_t;
