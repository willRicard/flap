#pragma once
#include "window.h"
#include <vulkan/vulkan.h>

VkSurfaceKHR window_vk_create_surface(const VkInstance instance);

// Return an array of required Vulkan instance extensions.
const char **window_vk_get_extensions(uint32_t *extensionCount);

VKAPI_ATTR VkBool32 VKAPI_CALL window_vk_debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

VKAPI_ATTR VkBool32 VKAPI_CALL window_vk_debug_report_callback(
    VkDebugReportFlagsEXT message_flags, VkDebugReportObjectTypeEXT obj_type,
    uint64_t src_object, size_t location, int32_t msg_code,
    const char *layer_prefix, const char *message, void *user_data);
