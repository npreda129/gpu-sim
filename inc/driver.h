#ifndef DRIVER_H
#define DRIVER_H

#include "core.h"
#include <stdint.h>
#include <SDL2/SDL.h>

struct Game {
  SDL_Window *window;
  SDL_Renderer *renderer;
};

int launch_threads(core_t * core, int nthreads, int binary_size, uint32_t *binary, int mem_size, uint8_t *mem);

int init_display(struct Game *game, int windowWidth, int windowHeight); 

#endif
