#ifndef FLAP_WINDOW_H
#define FLAP_WINDOW_H
#include <vulkan/vulkan.h>

/**
 * A window for rendering.
 */
void window_init();

void window_quit();

VkSurfaceKHR window_create_surface(VkInstance instance);

void window_update();

void window_fail_with_error(const char *message);

// Return an array of required Vulkan instance extensions.
const char **window_get_extensions(uint32_t *extensionCount);

int window_should_close();

float window_get_time();

int window_get_thrust();

#endif // FLAP_WINDOW_H
