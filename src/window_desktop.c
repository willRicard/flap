#include "window.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include <GLFW/glfw3.h>

#include "flap.h"

static GLFWwindow *window = NULL;
static int thrust = 0;
static int pause = 0;

int window_should_close() { return glfwWindowShouldClose(window); }

float window_get_time() { return (float)glfwGetTime(); }

int window_get_thrust() { return thrust; }

int window_get_pause() { return pause; }

void window_fail_with_error(const char *error) {
#ifdef _WIN32
  MessageBox(NULL, error, "Error", MB_ICONERROR | MB_OK);
#endif
  fputs(error, stderr);
  exit(EXIT_FAILURE);
}

static void error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error: %s\n", description);
  exit(EXIT_FAILURE);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    thrust = 1;
  } else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
    pause = 1;
  } else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void window_init() {
  if (!glfwInit()) {
    window_fail_with_error("An error occurred while initializing GLFW.");
  }

  glfwSetErrorCallback(error_callback);

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window = glfwCreateWindow(FLAP_WINDOW_WIDTH, FLAP_WINDOW_HEIGHT, "Flap", NULL,
                            NULL);
  glfwSetKeyCallback(window, key_callback);
}

VkSurfaceKHR window_create_surface(VkInstance instance) {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  glfwCreateWindowSurface(instance, window, NULL, &surface);
  return surface;
}

void window_quit() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void window_update() {
  thrust = 0;
  pause = 0;
  glfwPollEvents();
}

const char **window_get_extensions(uint32_t *extension_count) {
  return glfwGetRequiredInstanceExtensions(extension_count);
}

VKAPI_ATTR VkBool32 VKAPI_CALL window_debug_messenger_callback(
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

VKAPI_ATTR VkBool32 VKAPI_CALL window_debug_report_callback(
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
