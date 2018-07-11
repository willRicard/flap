#include "window.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "flap.h"

struct flap_Window {
  GLFWwindow *window;
};

void GLAPIENTRY flap_message_callback(GLenum source, GLenum type, GLuint id,
                                      GLenum severity, GLsizei length,
                                      const GLchar *message,
                                      const void *param) {

  fprintf(stderr, "OpenGL: %s\n", message);
}

static void error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error: %s\n", description);
}

static void resize_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

flap_Window *flap_window_new() {
  glfwInit();

  glfwSetErrorCallback(error_callback);

  flap_Window *window = (flap_Window *)malloc(sizeof(flap_Window));

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window->window = glfwCreateWindow(FLAP_WINDOW_WIDTH, FLAP_WINDOW_HEIGHT,
                                    FLAP_WINDOW_TITLE, NULL, NULL);

  glfwMakeContextCurrent(window->window);

  if (glewIsSupported("ARB_debug_output")) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(flap_message_callback, 0);
  }

  glfwSetKeyCallback(window->window, key_callback);
  glfwSetFramebufferSizeCallback(window->window, resize_callback);

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    printf("GLEW Error: %s\n", glewGetErrorString(err));
    exit(1);
  }

  return window;
}

void flap_window_free(flap_Window *window) {
  glfwDestroyWindow(window->window);
  glfwTerminate();
  free(window);
}

int flap_window_should_close(flap_Window *window) {
  return glfwWindowShouldClose(window->window);
}

int flap_window_thrust(flap_Window *window) {
  return (glfwGetKey(window->window, GLFW_KEY_SPACE) == GLFW_PRESS);
}

void flap_window_update(flap_Window *window) {
  glClearColor(0.53f, 0.81f, 0.92f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);
  glfwPollEvents();
}

void flap_window_render(flap_Window *window) {
  glfwSwapBuffers(window->window);
}
