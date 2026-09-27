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
    if (!fgets(input, MAX_INPUT - 1, stdin)) {
      fprintf(stderr, "Error reading input: %s\n", SDL_GetError());
      return 1;
    }
    if (!strcmp(input, "quit")) {
      return 0;
    }
  }
}
