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

  core_init(&core, WINDOW_WIDTH * WINDOW_HEIGHT);
  init_display(&game, WINDOW_WIDTH, WINDOW_HEIGHT);
  
  for (;;) {
    if (SDL_PollEvent(&(game.event)) && game.event.type == SDL_QUIT)
            break;
  }
  SDL_DestroyRenderer(game.renderer);
  SDL_DestroyWindow(game.window);
  SDL_Quit();
  return 0;
}
