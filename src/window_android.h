#ifndef FLAP_ANDROID_WINDOW_H_
#define FLAP_ANDROID_WINDOW_H_
#include "window.h"
#include <android_native_app_glue.h>

/*
 * Return the application structure needed to interface with the Android system.
 */
struct android_app *android_window_get_app();

#endif // FLAP_ANDROID_WINDOW_H_
