#include "window_desktop.h"
#include <stdio.h>
#include <stdlib.h>

#include "flap.h"

#ifdef _WIN32
#include <windows.h>
#endif

static GLFWwindow *window = NULL;

GLFWwindow *flapWindowDesktopGetWindow() { return window; }

static void errorCallback(int error, const char *description) {
  fprintf(stderr, "GLFW Error: %s\n", description);
  exit(EXIT_FAILURE);
}

static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                        int mods) {

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void flapWindowDesktopInit() {
  if (!glfwInit()) {
    flapWindowFailWithError("An error occurred while initializing GLFW.");
  }

  glfwSetErrorCallback(errorCallback);
}

void flapWindowDesktopCreateWindow() {
  window = glfwCreateWindow(FLAP_WINDOW_WIDTH, FLAP_WINDOW_HEIGHT, "Flap", NULL,
                            NULL);
  glfwSetKeyCallback(window, keyCallback);
}

void flapWindowQuit() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

int flapWindowShouldClose() { return glfwWindowShouldClose(window); }

float flapWindowGetTime() { return (float)glfwGetTime(); }

int flapWindowGetThrust() {
  return (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
}

void flapWindowFailWithError(const char *error) {
#ifdef _WIN32
  MessageBox(NULL, error, "Error", MB_ICONERROR | MB_OK);
#endif
  fputs(error, stderr);
  exit(EXIT_FAILURE);
}
