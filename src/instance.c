#include "instance.h"

#include "error.h"
#include "window.h"

VkInstance instance_create() {
  VkApplicationInfo app_info = {0};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Flap";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  uint32_t surface_extension_count = 0;
  const char **surface_extensions =
      window_get_extensions(&surface_extension_count);

  VkInstanceCreateInfo instance_info = {0};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledExtensionCount = surface_extension_count;
  instance_info.ppEnabledExtensionNames = surface_extensions;

  VkInstance instance = VK_NULL_HANDLE;
  error_check(vkCreateInstance(&instance_info, NULL, &instance),
              "vkCreateInstance");

  return instance;
}
