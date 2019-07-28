#include "window_android.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <android/log.h>
#include <android_native_app_glue.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

int main(void);

static struct android_app *flap_app;
static int window_ready = 0;
static int should_close = 0;
static int thrust = 0;
static int pause = 0;

struct android_app *android_window_get_app() {
  return flap_app;
}

static void on_app_cmd(struct android_app *app, int32_t cmd) {
  switch (cmd) {
  case APP_CMD_INIT_WINDOW:
    window_ready = 1;
    pause = 0;
    break;
  case APP_CMD_TERM_WINDOW:
    should_close = 1;
    break;
  default:
    break;
  }
}

static int32_t on_input_event(struct android_app *app, AInputEvent *event) {
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    int32_t action = AMotionEvent_getAction(event);
    if (action == AKEY_EVENT_ACTION_DOWN) {
      thrust = 1;
    }
    return 1;
  }
  return 0;
}

void android_main(struct android_app *app) {
  app->onAppCmd = on_app_cmd;
  app->onInputEvent = on_input_event;
  flap_app = app;

  while (!window_ready) {
    window_update();
  }
  main();
}

void window_init() {
  // No need to do anything: Android provides us with a window.
}

void window_quit() {
  // No need to do anything: Android provides us with a window.
}

void window_update() {
  thrust = 0;
  pause = 0;

  int numEvents = 0;
  struct android_poll_source *source = NULL;

  while (ALooper_pollAll(0, NULL, &numEvents, (void **)&source) >= 0) {
    if (source != NULL) {
      source->process(flap_app, source);
    }

    if (flap_app->destroyRequested != 0) {
      should_close = 1;
      return;
    }
  }
}

int window_should_close() { return should_close; }

float window_get_time() {
  static const float second = 1000000000.F;

  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return (time.tv_sec * second + (float)time.tv_nsec) / second;
}

int window_get_thrust() { return thrust; }

int window_get_pause() { return pause; }

/**
 * Display an error message through
 * an Android dialog then exit.
 */
void window_fail_with_error(const char *error) {
  JavaVM *vm = flap_app->activity->vm;
  JNIEnv *env = NULL;

  int env_status = (*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6);

  if (env_status == JNI_EDETACHED) {
    (*vm)->AttachCurrentThread(vm, &env, NULL);
  }

  jclass clazz = (*env)->GetObjectClass(env, flap_app->activity->clazz);

  jobject activity = flap_app->activity->clazz;

  jmethodID fail_with_error =
      (*env)->GetMethodID(env, clazz, "failWithError", "(Ljava/lang/String;)V");

  jstring error_string = (*env)->NewStringUTF(env, error);

  (*env)->CallVoidMethod(env, activity, fail_with_error, error_string);

  // Let the dialog run on its own thread.
  (*vm)->DetachCurrentThread(vm);
}

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
