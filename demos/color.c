#include <stdio.h>
#include <string.h>
#include "driver.h"
#include "core.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
// each simulated pixel is drawn as a PIXEL_SIZE x PIXEL_SIZE square, so the
// gpu only computes RENDER_WIDTH * RENDER_HEIGHT pixels
#define PIXEL_SIZE 4
#define RENDER_WIDTH (WINDOW_WIDTH / PIXEL_SIZE)
#define RENDER_HEIGHT (WINDOW_HEIGHT / PIXEL_SIZE)
#define MAX_INPUT 80

int main() {
  core_t core;
  struct Game game;
  char input[MAX_INPUT];
  FILE *kernel_file;
  uint32_t binary[MAX_B_SIZE];
  uint32_t pixel;
  uint8_t *dataMem = NULL;

  core_init(&core, RENDER_WIDTH * RENDER_HEIGHT);
  init_display(&game, WINDOW_WIDTH, WINDOW_HEIGHT);

  // load kernel from file TODO verify file exists
  kernel_file = fopen("build/color_kernel", "r");
  size_t binary_size = fread(binary, 4, MAX_B_SIZE, kernel_file);

  // launch threads. mem_size is 0 because this kernel needs no data section
  launch_threads(&core, 1, binary_size * 4, binary, 0, dataMem);

  //at this point, the pixels will be in core memory
  for (int x = 0; x < RENDER_WIDTH; x++) {
    for (int y = 0; y < RENDER_HEIGHT; y++) {
      memcpy(&pixel, &(core.sharedMem[4 * (y * RENDER_WIDTH + x)]), 4);
      SDL_SetRenderDrawColor(game.renderer, 
          (pixel >> 24) & 0xFF, (pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, 0xFF);
      SDL_Rect square = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};
      SDL_RenderFillRect(game.renderer, &square);
    }
  }

  SDL_RenderPresent(game.renderer);
  
  for (;;) {
    if (SDL_PollEvent(&(game.event)) && game.event.type == SDL_QUIT)
            break;
  }

  destroy_display(&game);
  return 0;
}
