#include "driver.h"
#include "trace.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// prints the kernel with each instruction's meaning, once per new binary
static void print_listing(uint32_t *binary, int count) {
  char text[64], meaning[96];

  printf("[gpu] loaded a new kernel: %d instructions (%d bytes)\n", count, count * 4);
  for (int pc = 0; pc < count; pc++) {
    trace_disassemble(binary[pc], text, sizeof(text));
    trace_explain(binary[pc], meaning, sizeof(meaning));
    printf("[gpu]   pc %3d  %08x  %-16s ; %s\n", pc, binary[pc], text, meaning);
  }
}

int launch_threads(core_t * core, int n, int binary_size, uint32_t *binary, int mem_size, uint8_t *mem) {
  static int runs = 0;
  core_stats_t *stats = &core->stats;
  int newBinary = binary_size != core->binarySize
    || memcmp(core->binaryMem, binary, binary_size);
  clock_t start = clock();

  runs++;
  if (trace_level >= 1) {
    printf("[gpu] ==== run %d: launching the kernel %d time%s on %d lanes ====\n",
        runs, n, n == 1 ? "" : "s", core->numLanes);
    if (newBinary) {
      print_listing(binary, binary_size / 4);
    }
    printf("[gpu] copying %d bytes of input from host to shared memory, zeroing every lane's registers\n",
        mem_size);
  }

  // TODO validate binary and mem size
  memcpy(core->binaryMem, binary, binary_size);
  core->binarySize = binary_size;
  // nowhere in shared memory should be left as garbage
  // since the threads may execute multiple times
  // building on previous runs' results
  // and the thread does not know if it has run before
  memcpy(core->sharedMem, mem, mem_size);

  // zero each lane's registers for the same reason
  for (int i = 0; i < core->numLanes; i++) {
    memset(core->lanes[i], 0, sizeof(lane_t));
    // set thread id
    *(uint32_t *)register_access((core->lanes[i]), 14) = i;
  }
  memset(stats, 0, sizeof(core_stats_t));

  // run the kernel n times. 
  // reset the core's RIP after each
  for (int i = 0; i < n; i++) {
    uint64_t issuedBefore = stats->instructions;

    core->tracing = trace_level >= 2 && i < trace_launches;
    if (core->tracing) {
      printf("[gpu] -- launch %d/%d, tracing every instruction%s --\n", i + 1, n,
          trace_level >= 3 ? " and its result in sample lanes" : "");
    }
    while (core_step(core))
      ; // do nothing
    if (trace_level >= 2) {
      printf("[gpu] launch %d/%d: %llu instructions issued, halted at pc %d\n", i + 1, n,
          (unsigned long long)(stats->instructions - issuedBefore), core->rip);
    }
    core->rip = 0;
  }
  core->tracing = 0;

  if (trace_level >= 1) {
    double seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("[gpu] run %d done. instructions issued: %llu (%.1f per launch)\n", runs,
        (unsigned long long)stats->instructions, (double)stats->instructions / n);
    printf("[gpu]   each issue does work in all %d lanes: %llu lane-instructions\n",
        core->numLanes, (unsigned long long)stats->laneOps);
    double kb = (stats->loads + stats->stores) * 4 / 1024.0;
    printf("[gpu]   memory traffic: %llu loads + %llu stores = %.1f %s; float math: %llu flops\n",
        (unsigned long long)stats->loads, (unsigned long long)stats->stores,
        kb < 1024 ? kb : kb / 1024, kb < 1024 ? "KB" : "MB", (unsigned long long)stats->flops);
    printf("[gpu]   simulated one lane at a time in %.3f s of host CPU = %.1f M lane-instructions/s\n",
        seconds, seconds > 0 ? stats->laneOps / seconds / 1e6 : 0);
    printf("[gpu]   real hardware runs the lanes side by side, one issue per clock: ~%llu ns at 1 GHz\n",
        (unsigned long long)stats->instructions);
  }
  return 0;
}

void init_display(struct Game *game, int windowWidth, int windowHeight) {
  if (SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
		exit(1);
  }

  if (SDL_CreateWindowAndRenderer(windowWidth, windowHeight, 0, &(game->window), &(game->renderer))) {
		fprintf(stderr, "Error creating window and renderer: %s\n", SDL_GetError());
		exit(1);
	}
}

void destroy_display(struct Game *game) {
  // TODO error handling
  SDL_DestroyRenderer(game->renderer);
  SDL_DestroyWindow(game->window);
  SDL_Quit();
}
