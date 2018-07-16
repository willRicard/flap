#include "init.h"

#include "rect.h"
#include "vk/renderer.h"
#include "window.h"

void flapInit() {
  flapWindowInit();
  flapRendererInit();
  flapRectInit();
}

void flapQuit() {
  flapRectQuit();
  flapRendererQuit();
  flapWindowQuit();
}
