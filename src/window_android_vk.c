#include "window.h"
#include "window_android.h"
#include "window_vk.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <android/log.h>
#include <android_native_app_glue.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>
#include <unistd.h>

int main(int argc, char **argv);

static struct android_app *flapApp;
static int windowReady = 0;
static int shouldClose = 0;
static int thrust = 0;

static void onAppCmd(struct android_app *app, int32_t cmd) {
  switch (cmd) {
  case APP_CMD_INIT_WINDOW:
    windowReady = 1;
    break;
  case APP_CMD_TERM_WINDOW:
    shouldClose = 1;
    break;
  case APP_CMD_LOST_FOCUS:
    break;
  }
}

static int32_t onInputEvent(struct android_app *app, AInputEvent *event) {
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    int32_t action = AMotionEvent_getAction(event);
    if (action == AKEY_EVENT_ACTION_DOWN) {
      thrust = 1;
    } else if (action == AMOTION_EVENT_ACTION_UP) {
      thrust = 0;
    }
    return 1;
  }
  return 0;
}

void android_main(struct android_app *app) {
  flapApp = app;
  app->onAppCmd = onAppCmd;
  app->onInputEvent = onInputEvent;

  while (!windowReady) {
      flapWindowUpdate();
  }

  main(0, NULL);
}

void flapWindowInit() {}

void flapWindowQuit() {}

int flapWindowShouldClose() { return shouldClose; }

float flapWindowGetTime() {
  return (float) clock() / CLOCKS_PER_SEC;
}

int flapWindowGetThrust() { return thrust; }

void flapWindowUpdate() {
    int numEvents;
    struct android_poll_source *source = NULL;

    while (ALooper_pollAll(0, NULL, &numEvents, (void **)&source) >= 0) {
        if (source != NULL) {
            source->process(flapApp, source);
        }

        if (flapApp->destroyRequested != 0) {
            shouldClose = 1;
            return;
        }
    }
}

void flapWindowRender() {}

void flapWindowFailWithError(const char *error) {
  __android_log_print(ANDROID_LOG_FATAL, "flap", "%s", error);
  pthread_exit(NULL);
}

struct android_app *flapGetApp() {
  return flapApp;
}

const char **flapWindowGetExtensions(uint32_t *extensionCount) {
  *extensionCount = 2;
  static const char *extensions[] = {"VK_KHR_surface",
                                     "VK_KHR_android_surface"};
  return extensions;
}

VkResult flapWindowCreateSurface(VkInstance instance, VkSurfaceKHR *surface) {
  VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
  surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
  surfaceCreateInfo.pNext = NULL;
  surfaceCreateInfo.flags = 0;
  surfaceCreateInfo.window = flapApp->window;

  return vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, NULL, surface);
}
