#include "window.h"
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

void window_init() {
  if (!glfwInit()) {
    window_fail_with_error("An error occurred while initializing GLFW.");
  }

  glfwSetErrorCallback(error_callback);

  if (!glfwVulkanSupported()) {
    window_fail_with_error("Vulkan is not supported on your platform.");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window = glfwCreateWindow(FLAP_WINDOW_WIDTH, FLAP_WINDOW_HEIGHT, "Flap", NULL,
                            NULL);
  glfwSetKeyCallback(window, key_callback);
}

VkSurfaceKHR window_create_surface(VkInstance instance) {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  glfwCreateWindowSurface(instance, window, NULL, &surface);
  return surface;
}

void window_quit() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void window_update() {
  // Sleep(64);
  glfwPollEvents();
}

const char **window_get_extensions(uint32_t *extensionCount) {
  return glfwGetRequiredInstanceExtensions(extensionCount);
}
