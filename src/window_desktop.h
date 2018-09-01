#ifndef FLAP_WINDOW_DESKTOP_H_
#define FLAP_WINDOW_DESKTOP_H_
#include "window.h"

// A GLFW window.

typedef struct GLFWwindow GLFWwindow;

void desktop_window_init();

void desktop_window_quit();

void desktop_window_create_window();

GLFWwindow *desktop_window_get_window();

#endif // FLAP_WINDOW_DESKTOP_H_
