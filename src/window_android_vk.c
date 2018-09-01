#include "window_android.h"
#include "window_vk.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

#include "renderer_vk.h"

void window_init() { renderer_init(); }

void window_quit() { renderer_quit(); }

void window_render() { renderer_render(); }

const char **vulkan_window_get_extensions(uint32_t *extensionCount) {
  *extensionCount = 2;
  static const char *extensions[] = {"VK_KHR_surface",
                                     "VK_KHR_android_surface"};
  return extensions;
}

VkResult vulkan_window_create_surface(VkInstance instance,
                                      VkSurfaceKHR *surface) {
  VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
  surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
  surfaceCreateInfo.window = android_window_get_app()->window;

  return vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, NULL, surface);
}
