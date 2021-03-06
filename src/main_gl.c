#include <stdlib.h>

#include <glad/glad.h>

#include "sprite_gl.h"
#include "window_gl.h"

#include "game.h"

int main(void) {
  window_init();

  sprite_init();

  glDisable(GL_DEPTH_TEST);

  game_init();

  glClearColor(0.53f, 0.81f, 0.92f, 1.f);

  while (!window_should_close()) {
    glClear(GL_COLOR_BUFFER_BIT);

    game_update();

    sprite_update();

    window_update();
  }

  sprite_quit();

  window_quit();
  return EXIT_SUCCESS;
}
