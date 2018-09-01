#include "window_desktop.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

static void resize_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void window_init() {
  desktop_window_init();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  desktop_window_create_window();

  GLFWwindow *window = desktop_window_get_window();

  glfwMakeContextCurrent(window);

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    printf("GLEW Error: %s\n", glewGetErrorString(err));
    exit(1);
  }

  glfwSetFramebufferSizeCallback(window, resize_callback);
}

void window_quit() { desktop_window_quit(); }

void window_update() {
  glClearColor(0.53f, 0.81f, 0.92f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);
  glfwPollEvents();
}

void window_render() {
  GLFWwindow *window = desktop_window_get_window();
  glfwSwapBuffers(window);
}
