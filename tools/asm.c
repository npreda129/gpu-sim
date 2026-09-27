#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "instruction.h"

#define MAX_LINE 80
#define INSTR_NAME_LEN 4
#define REG_NAME_LEN 2

int parse_regName(char *regName) {
  if (!strcmp(regName, "rF")) {
    return 15;
  }

  if (!strcmp(regName, "rT")) {
    return 14;
  }

  return atoi(regName + 1);
  fprintf(stderr, "Invalid register name\n");
  return -1;
}

int main(int argc, char ** argv) {
  FILE *read_file;
  FILE *write_file;
  char buffer[MAX_LINE];
  char instrName[MAX_LINE];
  char regNames[3][MAX_LINE];
  float immediate;
  uint32_t instrMachine;

  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Wrong number of args\n");
    return 1;
  }

  read_file = fopen(argv[1], "r");

  if (argc == 3) {
    write_file = fopen(argv[2], "w");
  }
  write_file = fopen("a.out", "w");

  while (fgets(buffer, MAX_LINE - 1, read_file)) {
    // discard comments
    if (buffer[0] == ';') {
      continue;
    }
    printf("Current Line: %s\n", buffer);
    // parse instruction name in order to determine format
    if (sscanf(buffer, "%s", instrName) < 1) {
      fprintf(stderr, "Failed to parse instruction name\n");
      return 1;
    }
    printf("Instruction Name: %s\n", instrName);

    // JUMP is not supported currently
    // TODO error on trying to write to r0, rT, rF
    if (!strcmp(instrName, "LI")) {
      if (sscanf(buffer, "%s %s %f", instrName, regNames[0], &immediate) < 2) {
        fprintf(stderr, "Failed to parse register name and immediate\n");
        return 1;
      }
      printf("Register: %s, Immediate: %f\n", regNames[0], immediate);
      
      instrMachine = (OP_LI << SHIFT_OP) + (parse_regName(regNames[0]) << SHIFT_RS) + immediate;
      fwrite(&instrMachine, sizeof(uint32_t), 1, write_file);
      continue;
    }

    // R format instruction
    if (sscanf(buffer, "%s %s %s %s", instrName, 
          regNames[0], regNames[1], regNames[2]) < 4) {
      fprintf(stderr, "Failed to parse register names\n");
      return 1;
    }
    printf("rS: %s, rT: %s, rD: %s\n", regNames[0], regNames[1], regNames[2]);
    
    if (!strcmp(instrName, "FMA")) {
      instrMachine = OP_FMA << SHIFT_OP;
    }
    if (!strcmp(instrName, "LW")) {
      instrMachine = OP_LW << SHIFT_OP;
    }
    if (!strcmp(instrName, "SW")) {
      instrMachine = OP_SW << SHIFT_OP;
    }
    if (!strcmp(instrName, "CMP")) {
      instrMachine = OP_CMP << SHIFT_OP;
    }
    if (!strcmp(instrName, "CMOV")) {
      instrMachine = OP_CMOV << SHIFT_OP;
    }
    if (!strcmp(instrName, "HALT")) {
      instrMachine = OP_HALT << SHIFT_OP;
    }
    if (!strcmp(instrName, "SHL")) {
      instrMachine = OP_SHL << SHIFT_OP;
    }
    if (!strcmp(instrName, "SHR")) {
      instrMachine = OP_SHR << SHIFT_OP;
    }
    instrMachine += (parse_regName(regNames[0]) << SHIFT_RS);
    instrMachine += (parse_regName(regNames[1]) << SHIFT_RT);
    instrMachine += (parse_regName(regNames[2]) << SHIFT_RD);
    // lower 16 bits will be garbage.
    fwrite(&instrMachine, sizeof(uint32_t), 1, write_file);

    printf("\n");
  }
  printf("Reached EOF\n");

  return 0;
}
