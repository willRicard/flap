#include "window.h"
#include <switch.h>

void window_update_gl();

static int thrust = 0;
static int pause = 0;

void window_update() {
  window_update_gl();

  thrust = 0;
  pause = 0;
  hidScanInput();
  u32 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
  if (kDown & KEY_A) {
    thrust = 1;
  } else if (kDown & KEY_PLUS) {
    pause = 1;
  }
}

void window_fail_with_error(const char *message) {
  ErrorApplicationConfig config;
  errorApplicationCreate(&config,
                         "Flap has crashed with an error (see details). Please "
                         "open an issue on github.com/willRicard/flap",
                         message);
  errorApplicationShow(&config);
}

int window_should_close() { return !appletMainLoop(); }

float window_get_time() { return 0.0f; }

int window_get_thrust() { return thrust; }

int window_get_pause() { return pause; }
