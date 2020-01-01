#include "window_android.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <android_native_app_glue.h>

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
