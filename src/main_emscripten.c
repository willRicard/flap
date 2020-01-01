#include <stdlib.h>
#include <emscripten/emscripten.h>

#include <glad/glad.h>

#include "sprite_gl.h"
#include "window_gl.h"

#include "game.h"

static void main_loop(void) {
  if (!window_should_close()) {
    glClear(GL_COLOR_BUFFER_BIT);

    game_update();

    sprite_update();

    window_update();
  } else {
    sprite_gl_quit();

    window_quit();
    exit(EXIT_SUCCESS);
  }
}

int main(void) {
  emscripten_set_main_loop(main_loop, 60, 0);

  window_init();

  // WebGL 1 does not support debug output
  if (glad_glDebugMessageCallback) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(window_gl_debug_message_callback, NULL);
  }

  sprite_gl_init();

  glDisable(GL_DEPTH_TEST);

  game_init();

  glClearColor(0.53f, 0.81f, 0.92f, 1.f);

  while (1) {
    main_loop();
  }

  return 0;
}
