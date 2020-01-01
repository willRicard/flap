#include "window_desktop.h"

#ifdef _WIN32
#include <windows.h>
#elif defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

#include <stdio.h>
#include <stdlib.h>

static const char *kFlapWindowTitle = "Flap";
static const int kFlapWindowWidth = 800;
static const int kFlapWindowHeight = 450;

GLFWwindow *window = NULL;
static int thrust = 0;
static int pause = 0;

int window_should_close() { return glfwWindowShouldClose(window); }

float window_get_time() { return (float)glfwGetTime(); }

int window_get_thrust() { return thrust; }

int window_get_pause() { return pause; }

void window_fail_with_error(const char *error) {
#ifdef _WIN32
  MessageBox(NULL, error, "Error", MB_ICONERROR | MB_OK);
#elif defined(__EMSCRIPTEN__)
  EM_ASM_(var str = UTF8ToString($0); alert(str);, error);
#endif
  fputs(error, stderr);
  exit(EXIT_FAILURE);
}

void window_desktop_error_callback(int error, const char *description) {
  window_fail_with_error(description);
}

void window_desktop_key_callback(GLFWwindow *window, int key, int scancode,
                                 int action, int mods) {
  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    thrust = 1;
  } else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
    pause = 1;
  } else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void window_desktop_mouse_button_callback(GLFWwindow *window, int button,
                                          int action, int mods) {
  if (action == GLFW_PRESS) {
    thrust = 1;
  }
}

void window_update() {
  thrust = 0;
  pause = 0;
  glfwPollEvents();
  // glfwSwapBuffers(window);
}

void window_quit() {
  glfwDestroyWindow(window);
  glfwTerminate();
}
