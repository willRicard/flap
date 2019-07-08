#ifndef SULFUR_ANDROID_WINDOW_H_
#define SULFUR_ANDROID_WINDOW_H_
#include <android_native_app_glue.h>
#include <sulfur/window.h>

/*
 * Return the application structure needed to interface with the Android system.
 */
struct android_app *android_window_get_app();

#endif // SULFUR_ANDROID_WINDOW_H_
