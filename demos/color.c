#include <stdio.h>
#include <string.h>
#include "driver.h"
#include "core.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MAX_INPUT 80

int main() {
  core_t core;
  struct Game game;
  char input[MAX_INPUT];
  FILE *kernel_file;
  uint32_t binary[MAX_B_SIZE];
  uint32_t pixel;
  uint8_t *dataMem;

  core_init(&core, WINDOW_WIDTH * WINDOW_HEIGHT);
  init_display(&game, WINDOW_WIDTH, WINDOW_HEIGHT);

  // load kernel from file
  kernel_file = fopen("build/color_kernel", "r");
  size_t binary_size = fread(binary, 4, MAX_B_SIZE, kernel_file);

  // launch threads. mem_size is 0 because this kernel needs no data section
  launch_threads(&core, 1, binary_size * 4, binary, 0, dataMem);

  //at this point, the pixels will be in core memory
  for (int x = 0; x < WINDOW_WIDTH; x++) {
    for (int y = 0; x < WINDOW_HEIGHT; y++) {
      pixel = ((uint32_t[WINDOW_WIDTH][WINDOW_HEIGHT]) core.sharedMem)[x][y];
      SDL_SetRenderDrawColor(game.renderer, 
          (pixel >> 24) & 0xFF, (pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, 0xFF);
      SDL_RenderDrawPoint(game.renderer, x, y);
    }
  }

  SDL_RenderPresent(renderer);
  
  for (;;) {
    if (SDL_PollEvent(&(game.event)) && game.event.type == SDL_QUIT)
            break;
  }

  destroy_display(&game);
  return 0;
}
