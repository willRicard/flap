#include "window.h"
#include "window_desktop.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "flap.h"
#include "renderer_vk.h"

static void resize_callback(GLFWwindow *window, int width, int height) {
  renderer_cleanup_swapchain();
  renderer_create_swapchain();
}

void window_init() {
  desktop_window_init();

  if (!glfwVulkanSupported()) {
    window_fail_with_error("Vulkan is not supported on your platform.");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  desktop_window_create_window();

  GLFWwindow *window = desktop_window_get_window();
  glfwSetFramebufferSizeCallback(window, resize_callback);

  renderer_init();
}

void window_quit() {
  renderer_quit();
  desktop_window_quit();
}

void window_update() { glfwPollEvents(); }

const char **vulkan_window_get_extensions(uint32_t *extensionCount) {
  return glfwGetRequiredInstanceExtensions(extensionCount);
}

VkResult vulkan_window_create_surface(VkInstance instance,
                                      VkSurfaceKHR *surface) {
  GLFWwindow *window = desktop_window_get_window();
  return glfwCreateWindowSurface(instance, window, NULL, surface);
}

void window_render() {
  renderer_render();
}
