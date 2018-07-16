#include "init.h"

#include "rect.h"
#include "window.h"

void flapInit() {
  flapWindowInit();
  flapRectInit();
}

void flapQuit() {
  flapRectQuit();
  flapWindowQuit();
}
