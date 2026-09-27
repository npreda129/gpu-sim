#ifndef TRACE_H
#define TRACE_H

#include <stdint.h>
#include <stddef.h>

// how much the simulator prints, set with environment variables:
//   GPU_TRACE=0  nothing
//   GPU_TRACE=1  (default) the kernel listing and a summary of every launch_threads call
//   GPU_TRACE=2  also every instruction the core issues, and a line per launch
//   GPU_TRACE=3  also what the instruction did in a few sample lanes
//   GPU_TRACE_LANES=n     how many sample lanes level 3 shows (default 2)
//   GPU_TRACE_LAUNCHES=n  how many launches per call levels 2 and 3 trace (default 1)
extern int trace_level;
extern int trace_lanes;
extern int trace_launches;

void trace_init(void);

// the name the assembler uses for register regNum
const char *trace_reg_name(int regNum);

// the instruction in assembler syntax, e.g. "FMA r4 r3 r10"
void trace_disassemble(uint32_t instruction, char *buf, size_t len);

// what the instruction does in plain words, e.g. "r10 += r4 * r3 (float)"
void trace_explain(uint32_t instruction, char *buf, size_t len);

// a register or memory word formatted for its type, e.g. "1.5" or "0x0000002a (42)"
void trace_format_value(uint32_t bits, int isFloat, char *buf, size_t len);

#endif
