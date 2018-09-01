#include "window_desktop.h"
#include <stdio.h>
#include <stdlib.h>

#include "flap.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <GLFW/glfw3.h>

static GLFWwindow *window = NULL;

int window_should_close() { return glfwWindowShouldClose(window); }

float window_get_time() { return (float)glfwGetTime(); }

int window_get_thrust() {
  return (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
}

void window_fail_with_error(const char *error) {
#ifdef _WIN32
  MessageBox(NULL, error, "Error", MB_ICONERROR | MB_OK);
#endif
  fputs(error, stderr);
  exit(EXIT_FAILURE);
}

GLFWwindow *desktop_window_get_window() { return window; }

static void error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error: %s\n", description);
  exit(EXIT_FAILURE);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void desktop_window_init() {
  if (!glfwInit()) {
    window_fail_with_error("An error occurred while initializing GLFW.");
  }

  glfwSetErrorCallback(error_callback);
}

void desktop_window_quit() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void desktop_window_create_window() {
  window = glfwCreateWindow(FLAP_WINDOW_WIDTH, FLAP_WINDOW_HEIGHT, "Flap", NULL,
                            NULL);
  glfwSetKeyCallback(window, key_callback);
}
