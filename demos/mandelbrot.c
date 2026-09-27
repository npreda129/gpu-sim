#include <stdio.h>
#include <string.h>
#include "driver.h"
#include "core.h"

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 300
#define MAX_INPUT 80

// the kernel runs one iteration per launch
#define ITERATIONS 15
#define PAN_FRACTION 0.1f

// the kernel reads these from offset 4 onward, in this order
struct mandelbrot_params {
  float left_bound;
  float top_bound;
  float multiplier; // distance between neighboring pixels in the complex plane
  float width;
  float inv_width;
  uint32_t color_step; // added to the pixel color once per iteration before escaping
};

// offset 0 of shared memory holds the framebuffer address. the framebuffer
// goes after the params so pixels don't overwrite them
#define FB_ADDR 64

int main() {
  core_t core;
  struct Game game;
  FILE *kernel_file;
  uint32_t binary[MAX_B_SIZE];
  uint32_t pixel;
  uint32_t fbAddr = FB_ADDR;
  uint8_t dataMem[sizeof(uint32_t) + sizeof(struct mandelbrot_params)];
  // start with the whole set on screen, centered at -0.5
  struct mandelbrot_params params = {
    .multiplier = 3.0f / WINDOW_WIDTH,
    .width = WINDOW_WIDTH,
    .inv_width = 1.0f / WINDOW_WIDTH,
    // each channel must stay <= 255 after ITERATIONS steps
    .color_step = ((80 / ITERATIONS) << 24) | ((160 / ITERATIONS) << 16) | ((255 / ITERATIONS) << 8),
  };
  float center_real = -0.5f;
  float center_imag = 0;
  int redraw = 1;

  core_init(&core, WINDOW_WIDTH * WINDOW_HEIGHT);
  init_display(&game, WINDOW_WIDTH, WINDOW_HEIGHT);

  // load kernel from file
  kernel_file = fopen("build/mandelbrot_kernel", "r");
  if (!kernel_file) {
    fprintf(stderr, "Could not open build/mandelbrot_kernel\n");
    return 1;
  }
  size_t binary_size = fread(binary, 4, MAX_B_SIZE, kernel_file);
  fclose(kernel_file);

  printf("up/down zoom, wasd pan\n");

  for (;;) {
    // poll events. SDL_PollEvent leaves game.event unchanged when there are
    // none, so only look at it when it returns 1
    while (SDL_PollEvent(&(game.event))) {
      if (game.event.type == SDL_QUIT) {
        destroy_display(&game);
        return 0;
      }
      if (game.event.type == SDL_KEYDOWN) {
        redraw = 1;
        switch (game.event.key.keysym.sym) {
          case SDLK_UP:
            params.multiplier *= 0.8f;
            break;
          case SDLK_DOWN:
            params.multiplier *= 1.25f;
            break;
          case SDLK_w:
            center_imag += PAN_FRACTION * WINDOW_HEIGHT * params.multiplier;
            break;
          case SDLK_s:
            center_imag -= PAN_FRACTION * WINDOW_HEIGHT * params.multiplier;
            break;
          case SDLK_a:
            center_real -= PAN_FRACTION * WINDOW_WIDTH * params.multiplier;
            break;
          case SDLK_d:
            center_real += PAN_FRACTION * WINDOW_WIDTH * params.multiplier;
            break;
          default:
            redraw = 0;
        }
      }
    }

    // a frame takes seconds, so only render when something changed
    if (!redraw) {
      SDL_Delay(16);
      continue;
    }
    redraw = 0;

    // zoom and pan around the center of the screen
    params.left_bound = center_real - WINDOW_WIDTH / 2.0f * params.multiplier;
    params.top_bound = center_imag + WINDOW_HEIGHT / 2.0f * params.multiplier;

    // define shared values once every frame after making changes based on user input
    memcpy(dataMem, &fbAddr, sizeof(uint32_t));
    memcpy(dataMem + 4, &params, sizeof(struct mandelbrot_params));

    // launch threads, one launch per iteration. the kernel writes every
    // pixel, so the framebuffer doesn't need to be part of dataMem
    printf("rendering...\n");
    launch_threads(&core, ITERATIONS, binary_size * 4, binary, sizeof(dataMem), dataMem);

    //at this point, the pixels will be in core memory
    for (int x = 0; x < WINDOW_WIDTH; x++) {
      for (int y = 0; y < WINDOW_HEIGHT; y++) {
        memcpy(&pixel, &(core.sharedMem[fbAddr + 4 * (y * WINDOW_WIDTH + x)]), 4);
        SDL_SetRenderDrawColor(game.renderer,
            (pixel >> 24) & 0xFF, (pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, 0xFF);
        SDL_RenderDrawPoint(game.renderer, x, y);
      }
    }

    SDL_RenderPresent(game.renderer);
    printf("done\n");
  }
}
