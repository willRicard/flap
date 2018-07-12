#include "window.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "flap.h"

static GLFWwindow *window;

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

void flap_window_init() {
  glfwInit();

  glfwSetErrorCallback(error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(FLAP_WINDOW_WIDTH, FLAP_WINDOW_HEIGHT,
                                    FLAP_WINDOW_TITLE, NULL, NULL);

  glfwMakeContextCurrent(window);

  if (glewIsSupported("ARB_debug_output")) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(flap_message_callback, 0);
  }

  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, resize_callback);

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    printf("GLEW Error: %s\n", glewGetErrorString(err));
    exit(1);
  }
}

void flap_window_quit() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

int flap_window_should_close() {
  return glfwWindowShouldClose(window);
}

int flap_window_thrust() {
  return (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
}

void flap_window_update() {
  glClearColor(0.53f, 0.81f, 0.92f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);
  glfwPollEvents();
}

void flap_window_render() {
  glfwSwapBuffers(window);
}
