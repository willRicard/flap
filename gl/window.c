#include "window.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "flap.h"

static GLFWwindow *window;

void GLAPIENTRY messageCallback(GLenum source, GLenum type, GLuint id,
                                GLenum severity, GLsizei length,
                                const GLchar *message, const void *param) {
  fprintf(stderr, "OpenGL: %s\n", message);
}

static void errorCallback(int error, const char *description) {
  fprintf(stderr, "GLFW Error: %s\n", description);
}

static void resizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                        int mods) {

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void flapWindowInit() {
  glfwInit();

  glfwSetErrorCallback(errorCallback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(FLAP_WINDOW_WIDTH, FLAP_WINDOW_HEIGHT,
                            FLAP_WINDOW_TITLE, NULL, NULL);

  glfwMakeContextCurrent(window);
  
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    printf("GLEW Error: %s\n", glewGetErrorString(err));
    exit(1);
  }

  if (glewIsSupported("ARB_debug_output")) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(messageCallback, 0);
  }

  glfwSetKeyCallback(window, keyCallback);
  glfwSetFramebufferSizeCallback(window, resizeCallback);
}

void flapWindowQuit() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

int flapWindowShouldClose() { return glfwWindowShouldClose(window); }

int flapWindowThrust() {
  return (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
}

void flapWindowUpdate() {
  glClearColor(0.53f, 0.81f, 0.92f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);
  glfwPollEvents();
}

void flapWindowRender() { glfwSwapBuffers(window); }
