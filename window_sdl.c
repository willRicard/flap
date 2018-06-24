#include "window.h"

#include <SDL2/SDL.h>

#include "flap.h"

struct flap_Window {
  SDL_Window *window;
  SDL_GLContext ctx;
}

flap_Window *
flap_window_new() {
  SDL_Init(SDL_INIT_VIDEO);

  flap_Window *window = (flap_Window *)malloc(sizeof(flap_Window));

  window->window = SDL_CreateWindow(FLAP_WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED, FLAP_WINDOW_WIDTH,
                                    FLAP_WINDOW_HEIGHT,
                                    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  glClearColor(0, 0, 255, 255);

  window->ctx = SDL_GL_CreateContext(window->window);

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    printf("GLEW Error: %s\n", glewGetErrorString(err));
    exit(1);
  }

  return window;
}

void flap_window_free(flap_Window *window) {
  SDL_GL_DeleteContext(window->ctx);
  SDL_DestroyWindow(window->window);
  SDL_Quit();
}

void flap_window_render(flap_Window *window) {
  SDL_GL_SwapWindow(window->window);
}
