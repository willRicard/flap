#include "window_android.h"

#include <EGL/egl.h>
#include <android/log.h>
#include <glad/glad.h>

static EGLDisplay display = EGL_NO_DISPLAY;
static EGLSurface surface = EGL_NO_SURFACE;
static EGLContext context = EGL_NO_CONTEXT;

void window_init() {
  display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglInitialize(display, NULL, NULL);

  static const EGLint attribs[] = {EGL_RENDERABLE_TYPE,
                                   EGL_OPENGL_ES2_BIT,
                                   EGL_SURFACE_TYPE,
                                   EGL_WINDOW_BIT,
                                   EGL_BLUE_SIZE,
                                   8,
                                   EGL_GREEN_SIZE,
                                   8,
                                   EGL_RED_SIZE,
                                   8,
                                   EGL_DEPTH_SIZE,
                                   24,
                                   EGL_NONE};

  EGLint num_configs = 0;
  EGLConfig config;
  eglChooseConfig(display, attribs, &config, 1, &num_configs);
  if (num_configs == 0) {
    window_fail_with_error("Failed to initialize OpenGL ES 2.0");
  }

  struct android_app *app = android_window_get_app();
  surface = eglCreateWindowSurface(display, config, app->window, NULL);
  if (surface == EGL_NO_SURFACE) {
    window_fail_with_error("Failed to create EGL surface");
  }

  static const EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2,
                                           EGL_NONE};

  context = eglCreateContext(display, config, NULL, context_attribs);
  if (context == EGL_NO_CONTEXT) {
    window_fail_with_error("Failed to create EGL context");
  }

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
    window_fail_with_error("eglMakeCurrent failed!");
  }

  int ok = gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress);
}

void window_quit() {
  eglDestroyContext(display, context);
  eglDestroySurface(display, surface);
  eglTerminate(display);
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
