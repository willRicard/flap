#include "window.h"
#include "window_desktop.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "flap.h"
#include "renderer_vk.h"

static void resizeCallback(GLFWwindow *window, int width, int height) {
  flapRendererCleanupSwapchain();
  flapRendererCreateSwapchain();
}

void flapWindowInit() {
  flapWindowDesktopInit();

  if (!glfwVulkanSupported()) {
    flapWindowFailWithError("Vulkan is not supported on your platform.");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  flapWindowDesktopCreateWindow();

  GLFWwindow *window = flapWindowDesktopGetWindow();
  glfwSetFramebufferSizeCallback(window, resizeCallback);
}

void flapWindowUpdate() { glfwPollEvents(); }

const char **flapWindowGetExtensions(uint32_t *extensionCount) {
  return glfwGetRequiredInstanceExtensions(extensionCount);
}

VkResult flapWindowCreateSurface(VkInstance instance, VkSurfaceKHR *surface) {
  GLFWwindow *window = flapWindowDesktopGetWindow();
  return glfwCreateWindowSurface(instance, window, NULL, surface);
}

void flapWindowRender() {}
