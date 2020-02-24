#include <stdlib.h>

#include <glad/glad.h>

#include "sprite_gl.h"
#include "window_gl.h"

#include "game.h"

int main(void) {
  window_init();

  if (glad_glDebugMessageCallback) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(window_gl_debug_message_callback, NULL);
  }

  sprite_gl_init();

  glDisable(GL_DEPTH_TEST);

  game_init();

  glClearColor(0.53f, 0.81f, 0.92f, 1.f);

  while (!window_should_close()) {
    glClear(GL_COLOR_BUFFER_BIT);

    game_update();

    sprite_update();

    window_update();
  }

  sprite_gl_quit();

  window_quit();
  return EXIT_SUCCESS;
}
