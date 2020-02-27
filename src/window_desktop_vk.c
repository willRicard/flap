#include "window_desktop.h"
#include "window_vk.h"

#include <stdio.h>

#include <GLFW/glfw3.h>

void window_init() {
  if (!glfwInit()) {
    window_fail_with_error("An error occurred while initializing GLFW.");
  }

  glfwSetErrorCallback(window_desktop_error_callback);

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window = glfwCreateWindow(FLAP_WINDOW_WIDTH, FLAP_WINDOW_HEIGHT,
                            FLAP_WINDOW_TITLE, NULL, NULL);
  glfwSetKeyCallback(window, window_desktop_key_callback);
}

VkSurfaceKHR window_vk_create_surface(const VkInstance instance) {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  glfwCreateWindowSurface(instance, window, NULL, &surface);
  return surface;
}

const char **window_vk_get_extensions(uint32_t *extension_count) {
  return glfwGetRequiredInstanceExtensions(extension_count);
}

VKAPI_ATTR VkBool32 VKAPI_CALL window_vk_debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
    void *user_data) {
  const char *severity_label = NULL;
  switch (message_severity) {
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    severity_label = "VERBOSE";
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    severity_label = "INFO";
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    severity_label = "WARNING";
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    severity_label = "ERROR";
    break;
  default:
    severity_label = "DEBUG";
    break;
  }

  printf("%s: [%s] Code %i : %s\n", severity_label,
         callback_data->pMessageIdName, callback_data->messageIdNumber,
         callback_data->pMessage);

  return VK_FALSE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL window_vk_debug_report_callback(
    VkDebugReportFlagsEXT message_flags, VkDebugReportObjectTypeEXT object_type,
    uint64_t src_object, size_t location, int32_t message_code,
    const char *layer_prefix, const char *message, void *user_data) {
  if (message_flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    printf("ERROR: [%s] Code %i : %s", layer_prefix, message_code, message);
  } else if (message_flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    printf("WARNING: [%s] Code %i : %s", layer_prefix, message_code, message);
  } else if (message_flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
    printf("PERFORMANCE WARNING: [%s] Code %i : %s", layer_prefix, message_code,
           message);
  } else if (message_flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
    printf("INFO: [%s] Code %i : %s", layer_prefix, message_code, message);
  } else if (message_flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
    printf("DEBUG: [%s] Code %i : %s", layer_prefix, message_code, message);
  }

  return VK_FALSE;
}
