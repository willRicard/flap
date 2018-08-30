#ifndef FLAP_VK_WINDOW_H_
#define FLAP_VK_WINDOW_H_

#include <vulkan/vulkan.h>

const char **flapWindowGetExtensions(uint32_t *extensionCount);

VkResult flapWindowCreateSurface(VkInstance instance, VkSurfaceKHR *surface);

#endif // FLAP_VK_WINDOW_H_
