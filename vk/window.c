#include "window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "vk/renderer.h"

static GLFWwindow *window;
static double lastUpdate = 0.0;

static void errorCallback(int error, const char *description) {
  fprintf(stderr, "GLFW Error: %s\n", description);
}

static void resizeCallback(GLFWwindow *window, int width, int height) {
  flapRendererCleanupSwapchain();
  flapRendererCreateSwapchain();
}

static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                        int mods) {

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void flapWindowInit() {
  if (!glfwInit()) {
    fputs("An error occurred while initializing GLFW.", stderr);
    exit(EXIT_FAILURE);
  }
  if (!glfwVulkanSupported()) {
    fputs("Vulkan is not supported on your platform.", stderr);
    exit(EXIT_FAILURE);
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(800, 450, "Flap", NULL, NULL);

  glfwSetKeyCallback(window, keyCallback);
}

void flapWindowQuit() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

const char **flapWindowGetExtensions(uint32_t *extensionCount) {
  return glfwGetRequiredInstanceExtensions(extensionCount);
}

VkResult flapWindowCreateSurface(VkInstance instance, VkSurfaceKHR *surface) {
  return glfwCreateWindowSurface(instance, window, NULL, surface);
}

int flapWindowShouldClose() { return glfwWindowShouldClose(window); }

double flapWindowGetTime() { return glfwGetTime(); }

int flapWindowThrust() {
  return (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
}

void flapWindowUpdate() { glfwPollEvents(); }

void flapWindowRender() {
  double lastUpdate = glfwGetTime();
  double nextUpdate = lastUpdate + 0.16;

  flapRendererRender();
}
