#ifndef FLAP_FLAP_H
#define FLAP_FLAP_H

#define VK_CHECK(fn, err_msg)                                                  \
  if (fn != VK_SUCCESS) {                                                      \
    window_fail_with_error(err_msg);                                           \
    exit(EXIT_FAILURE);                                                        \
  }

// Window
#define FLAP_WINDOW_TITLE "Flap"
#define FLAP_WINDOW_WIDTH 800
#define FLAP_WINDOW_HEIGHT 450

// Physics
#define FLAP_GRAVITY 2.0f
#define FLAP_THRUST -0.75f
#define FLAP_SCROLL_SPEED -0.24f
#define FLAP_FALL_INITIAL_SPEED 0.1f

// Bird
#define FLAP_BIRD_X -0.75f
#define FLAP_BIRD_Y -0.5f
#define FLAP_BIRD_WIDTH 0.08f
#define FLAP_BIRD_HEIGHT 0.14f

extern const float FLAP_BIRD_COLOR[];

// Pipes
#define FLAP_NUM_PIPES 4
#define FLAP_PIPE_WIDTH 0.12f
#define FLAP_PIPE_MIN_HEIGHT 0.25f
#define FLAP_PIPE_MAX_HEIGHT 1.0f
#define FLAP_PIPE_INITIAL_GAP 0.64f
#define FLAP_PIPE_STEP 0.5f

extern const float FLAP_PIPE_COLOR[];

#endif // FLAP_FLAP_H
