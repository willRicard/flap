#include <3ds.h>
#include <citro3d.h>
#include <stdio.h>

#define DISPLAY_TRANSFER_FLAGS                                                 \
  (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) |                       \
   GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |    \
   GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |                              \
   GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

static int thrust = 0;
static int pause = 0;

static C3D_RenderTarget *target = NULL;

static DVLB_s *vshader_dvlb = NULL;
static shaderProgram_s program = {0};

void window_init() {
  gfxInitDefault();
  target =
      C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
  C3D_RenderTargetSetOutput(target, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
  printf("[ok] init window\n");
}

void window_quit() { gfxExit(); }

void window_update() {
  thrust = 0;
  pause = 0;

  gspWaitForVBlank();
  printf("[ok] vblank\n");
  gfxSwapBuffers();
  printf("[ok] swap\n");
  hidScanInput();
  printf("[ok] hid\n");

  u32 kDown = hidKeysDown();

  if (kDown & KEY_A) {
    thrust = 1;
  }

  printf("[ok] input\n");

  C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
  printf("[ok] frame begin\n");

  C3D_RenderTargetClear(target, C3D_CLEAR_ALL, 0x68B0D8FF, 0);
  printf("[ok] clear\n");
  C3D_FrameDrawOn(target);
  printf("[ok] draw on\n");

  printf("[ok] window\n");
}

int window_should_close() { return !aptMainLoop(); }

float window_get_time() { return 0.F; }

int window_get_thrust() { return thrust; }

int window_get_pause() { return pause; }

void window_fail_with_error(const char *error) {
  u32 apt_buf[32] = {0};

  aptLaunchLibraryApplet(APPID_ERROR, apt_buf, sizeof(apt_buf), 0);
}
