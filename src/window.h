#ifndef FLAP_WINDOW_H_
#define FLAP_WINDOW_H_
#include <vulkan/vulkan.h>

/*
 * A window for rendering.
 */

void window_init();

void window_quit();

int window_should_close();

float window_get_time();

int window_get_thrust();

void window_update();

void window_render();

void window_fail_with_error(const char *error);

// Return an array of required extensions.
const char **window_get_extensions(uint32_t *extensionCount);

// Create a Vulkan surface for presentation.
VkResult window_create_surface(VkInstance instance, VkSurfaceKHR *surface);

#endif // FLAP_WINDOW_H_
