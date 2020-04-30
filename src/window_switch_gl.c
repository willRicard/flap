#include "window_gl.h"

#include <switch.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

static EGLDisplay display = EGL_NO_DISPLAY;
static EGLContext context = EGL_NO_CONTEXT;
static EGLSurface surface = EGL_NO_SURFACE;

void window_init() {
  display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (display == EGL_NO_DISPLAY) {
    window_fail_with_error("EGL: Could not connect to display");
  }

  eglInitialize(display, NULL, NULL);

  if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
    window_fail_with_error("EGL: Could not set graphics API");
  }

  EGLConfig config;
  EGLint num_configs = 0;
  static const EGLint framebuffer_attributes[] = {EGL_RENDERABLE_TYPE,
                                                  EGL_OPENGL_BIT,
                                                  EGL_RED_SIZE,
                                                  8,
                                                  EGL_GREEN_SIZE,
                                                  8,
                                                  EGL_BLUE_SIZE,
                                                  8,
                                                  EGL_ALPHA_SIZE,
                                                  8,
                                                  EGL_DEPTH_SIZE,
                                                  24,
                                                  EGL_STENCIL_SIZE,
                                                  8,
                                                  EGL_NONE};
  eglChooseConfig(display, framebuffer_attributes, &config, 1, &num_configs);
  if (num_configs == 0) {
    window_fail_with_error("EGL: No suitable configuration found");
  }

  NWindow *window = nwindowGetDefault();
  surface = eglCreateWindowSurface(display, config, window, NULL);
  if (surface == EGL_NO_SURFACE) {
    window_fail_with_error("EGL: Could not create window surface");
  }

  static const EGLint context_attributes[] = {
      EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,
      EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
      EGL_CONTEXT_MAJOR_VERSION_KHR,
      4,
      EGL_CONTEXT_MINOR_VERSION_KHR,
      3,
      EGL_NONE};

  context =
      eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
  if (context == EGL_NO_CONTEXT) {
    window_fail_with_error("EGL: Could not create context");
  }

  eglMakeCurrent(display, surface, surface, context);

  if (!gladLoadGL()) {
    window_fail_with_error("GLAD: Could not load OpenGL");
  }
}

void window_quit() {
  eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglDestroyContext(display, context);
  eglDestroySurface(display, surface);
  eglTerminate(display);
}

void window_update_gl() { eglSwapBuffers(display, surface); }
