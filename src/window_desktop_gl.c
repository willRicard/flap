#include "window.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

static void resizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void flapWindowInit() {
  flapWindowDesktopInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  flapWindowDesktopCreateWindow();

  GLFWwindow *window = flapWindowDesktopGetWindow();

  glfwMakeContextCurrent(window);

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    printf("GLEW Error: %s\n", glewGetErrorString(err));
    exit(1);
  }


  glfwSetFramebufferSizeCallback(window, resizeCallback);
}

void flapWindowUpdate() {
  glClearColor(0.53f, 0.81f, 0.92f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);
  glfwPollEvents();
}

void flapWindowRender() {
  GLFWwindow *window = flapWindowDesktopGetWindow();
  glfwSwapBuffers(window);
}
