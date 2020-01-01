#include <android/log.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

const char **window_vk_get_extensions(uint32_t *extension_count) {
  *extension_count = 2;
  static const char *extensions[] = {"VK_KHR_surface",
                                     "VK_KHR_android_surface"};
  return extensions;
}

VkSurfaceKHR window_vk_create_surface(const VkInstance instance) {
  VkAndroidSurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
  surface_info.window = android_window_get_app()->window;

  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkResult result =
      vkCreateAndroidSurfaceKHR(instance, &surface_info, NULL, &surface);
  if (result != VK_SUCCESS) {
    window_fail_with_error(
        "Error creating window surface: vkCreateAndroidSurfaceKHR");
  }
  return surface;
}

VKAPI_ATTR VkBool32 VKAPI_CALL window_vk_debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
    void *user_data) {
  switch (message_severity) {
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    __android_log_print(ANDROID_LOG_VERBOSE, "Flap", "[%s] Code %i : %s\n",
                        callback_data->pMessageIdName,
                        callback_data->messageIdNumber,
                        callback_data->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    __android_log_print(ANDROID_LOG_INFO, "Flap", "[%s] Code %i : %s\n",
                        callback_data->pMessageIdName,
                        callback_data->messageIdNumber,
                        callback_data->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    __android_log_print(ANDROID_LOG_WARN, "Flap", "[%s] Code %i : %s\n",
                        callback_data->pMessageIdName,
                        callback_data->messageIdNumber,
                        callback_data->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    __android_log_print(ANDROID_LOG_ERROR, "Flap", "[%s] Code %i : %s\n",
                        callback_data->pMessageIdName,
                        callback_data->messageIdNumber,
                        callback_data->pMessage);
    break;
  default:
    __android_log_print(ANDROID_LOG_DEBUG, "Flap", "[%s] Code %i : %s\n",
                        callback_data->pMessageIdName,
                        callback_data->messageIdNumber,
                        callback_data->pMessage);
    break;
  }

  return VK_FALSE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL window_vk_debug_report_callback(
    VkDebugReportFlagsEXT message_flags, VkDebugReportObjectTypeEXT object_type,
    uint64_t src_object, size_t location, int32_t message_code,
    const char *layer_prefix, const char *message, void *user_data) {
  if (message_flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    __android_log_print(ANDROID_LOG_ERROR, "Flap", "ERROR: [%s] Code %i : %s",
                        layer_prefix, message_code, message);
  } else if (message_flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    __android_log_print(ANDROID_LOG_WARN, "Flap", "WARNING: [%s] Code %i : %s",
                        layer_prefix, message_code, message);
  } else if (message_flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
    __android_log_print(ANDROID_LOG_WARN, "Flap",
                        "PERFORMANCE WARNING: [%s] Code %i : %s", layer_prefix,
                        message_code, message);
  } else if (message_flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
    __android_log_print(ANDROID_LOG_INFO, "Flap", "INFO: [%s] Code %i : %s",
                        layer_prefix, message_code, message);
  } else if (message_flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
    __android_log_print(ANDROID_LOG_VERBOSE, "Flap", "DEBUG: [%s] Code %i : %s",
                        layer_prefix, message_code, message);
  }

  return VK_FALSE;
}
