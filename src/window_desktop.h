#pragma once
#include "window.h"

// Including GLFW on Emscripten redefines all OpenGL symbols and macros.
#ifdef __EMSCRIPTEN__
#include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>

#define FLAP_WINDOW_TITLE "Flap"
#define FLAP_WINDOW_WIDTH 800
#define FLAP_WINDOW_HEIGHT 450

extern GLFWwindow *window;

void window_desktop_error_callback(int error, const char *description);

void window_desktop_key_callback(GLFWwindow *window, int key, int scancode,
                                 int action, int mods);

void window_desktop_mouse_button_callback(GLFWwindow *window, int button,
                                          int action, int mods);
