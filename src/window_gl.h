#pragma once

#include "window_desktop.h"
#include <glad/glad.h>

void APIENTRY window_gl_debug_message_callback(GLenum source, GLenum type,
                                               GLuint id, GLenum severity,
                                               GLsizei length,
                                               const GLchar *message,
                                               const void *user_param);
