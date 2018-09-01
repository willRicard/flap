#ifndef FLAP_VK_WINDOW_H_
#define FLAP_VK_WINDOW_H_
#include "window.h"

#include <vulkan/vulkan.h>

// A window with extra methods to enable Vulkan.

/*
 * Return an array of required extensions.
 */
const char **vulkan_window_get_extensions(uint32_t *extensionCount);

/*
 * Create a Vulkan surface for presentation.
 */
VkResult vulkan_window_create_surface(VkInstance instance, VkSurfaceKHR *surface);

#endif // FLAP_VK_WINDOW_H_
