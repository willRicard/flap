#include <stdlib.h>

#include <glad/glad.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "sprite_gl.h"
#include "window_gl.h"

#include "game.h"

static void main_loop(void) {
  if (!window_should_close()) {
    glClear(GL_COLOR_BUFFER_BIT);

    game_update();

    sprite_update();

    window_update();
    glfwSwapBuffers(window);
  } else {
    sprite_gl_quit();

    window_quit();
    exit(EXIT_SUCCESS);
  }
}

int main(void) {
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 60, 0);
#endif

  window_init();

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    window_fail_with_error("Failed to load OpenGL!");
  }

  // WebGL 1 does not support debug output
#if !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
  if (glad_glDebugMessageCallback) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(window_gl_debug_message_callback, NULL);
  }
#endif

  sprite_gl_init();

  glDisable(GL_DEPTH_TEST);

  game_init();

  glClearColor(0.53f, 0.81f, 0.92f, 1.f);

#ifndef __EMSCRIPTEN__
  while (1) {
    main_loop();
  }
#endif

  return 0;
}
