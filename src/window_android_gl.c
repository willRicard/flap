#include <android/log.h>
#include <glad/glad.h>

#include "window.h"

void window_init() {
  if (!gladLoadGL()) {
    window_fail_with_error("Failed to load OpenGL!");
  }
}

GLAPI void APIENTRY window_gl_debug_message_callback(GLenum source, GLenum type,
                                                     GLuint id, GLenum severity,
                                                     GLsizei length,
                                                     const GLchar *message,
                                                     const void *user_param) {
  switch (type) {
  case GL_DEBUG_TYPE_ERROR:
    __android_log_print(ANDROID_LOG_ERROR, "Flap", "ERROR: %s", message);
    break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    __android_log_print(ANDROID_LOG_WARN, "Flap", "DEPRECATED: %s", message);
    break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    __android_log_print(ANDROID_LOG_WARN, "Flap", "UNDEFINED BEHAVIOR: %s",
                        message);
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    __android_log_print(ANDROID_LOG_WARN, "Flap", "PERF: %s", message);
    break;
  case GL_DEBUG_TYPE_PORTABILITY:
    __android_log_print(ANDROID_LOG_WARN, "Flap", "PORTABILITY: %s", message);
    break;
  default:
    __android_log_print(ANDROID_LOG_DEBUG, "Flap", "OTHER: %s", message);
  }
}
