#ifndef FLAP_WINDOW_H
#define FLAP_WINDOW_H
#include <vulkan/vulkan.h>

/**
 * A window for rendering.
 */
void window_init();

void window_quit();

VkSurfaceKHR window_create_surface(const VkInstance instance);

void window_update();

void window_fail_with_error(const char *message);

// Return an array of required Vulkan instance extensions.
const char **window_get_extensions(uint32_t *extensionCount);

int window_should_close();

float window_get_time();

int window_get_thrust();

int window_get_pause();

VKAPI_ATTR VkBool32 VKAPI_CALL window_debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

VKAPI_ATTR VkBool32 VKAPI_CALL window_debug_report_callback(
    VkDebugReportFlagsEXT message_flags, VkDebugReportObjectTypeEXT obj_type,
    uint64_t src_object, size_t location, int32_t msg_code,
    const char *layer_prefix, const char *message, void *user_data);

#endif // FLAP_WINDOW_H
