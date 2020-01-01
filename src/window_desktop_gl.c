#include "window_desktop.h"
#include "window_gl.h"

#include <stdio.h>

static void on_resize(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void window_init() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    window_fail_with_error("Failed to load OpenGL!");
  }
  if (!glfwInit()) {
    window_fail_with_error("An error occurred while initializing GLFW.");
  }

  glfwSetErrorCallback(window_desktop_error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

  window = glfwCreateWindow(FLAP_WINDOW_WIDTH, FLAP_WINDOW_HEIGHT,
                            FLAP_WINDOW_TITLE, NULL, NULL);

  glfwMakeContextCurrent(window);

  glfwSwapInterval(1);

  glfwSetWindowSizeCallback(window, on_resize);

  glfwSetKeyCallback(window, window_desktop_key_callback);
  glfwSetMouseButtonCallback(window, window_desktop_mouse_button_callback);
}

GLAPI void APIENTRY window_gl_debug_message_callback(GLenum source, GLenum type,
                                                     GLuint id, GLenum severity,
                                                     GLsizei length,
                                                     const GLchar *message,
                                                     const void *user_param) {
  switch (type) {
  case GL_DEBUG_TYPE_ERROR:
    printf("ERROR: %s\n", message);
    break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    printf("DEPRECATED: %s\n", message);
    break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    printf("UNDEFINED BEHAVIOR: %s\n", message);
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    printf("PERF: %s\n", message);
    break;
  case GL_DEBUG_TYPE_PORTABILITY:
    printf("PORTABILITY: %s\n", message);
    break;
  default:
    printf("OTHER: %s\n", message);
  }
}
