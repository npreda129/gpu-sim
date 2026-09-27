#include "driver.h" 

int launch_threads(core_t * core, int nthreads, int binary_size, uint32_t *binary, int mem_size, uint8_t *mem) {
  // TODO validate binary and mem size
  memcpy(core->binaryMem, binary, binary_size);
  memcpy(core->sharedMem, mem, mem_size);
}

int init_display(struct Game *game, int windowWidth, int windowHeight) {
  if (SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
		return 1;
  }

  if (SDL_CreateWindowAndRenderer(windowWidth, windowHeight, 0, &(game->window), &(game->renderer))) {
		fprintf(stderr, "Error creating window and renderer: %s\n", SDL_GetError());
		return 1;
	}
  return 0;
}
