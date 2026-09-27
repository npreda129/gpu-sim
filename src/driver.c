#include "driver.h" 

int launch_threads(core_t * core, int n, int binary_size, uint32_t *binary, int mem_size, uint8_t *mem) {
  // TODO validate binary and mem size
  memcpy(core->binaryMem, binary, binary_size);
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

  // run the kernel n times. 
  // reset the core's RIP after each
  for (int i = 0; i < n; i++) {
    while (core_step(core))
      ; // do nothing
    core->rip = 0;
  }
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
