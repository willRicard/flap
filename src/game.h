#ifndef FLAP_GAME_H
#define FLAP_GAME_H

#define FLAP_SCREEN_TOP -1.F
#define FLAP_SCREEN_BOTTOM 1.F
#define FLAP_SCREEN_HEIGHT 2.F
#define FLAP_SCREEN_LEFT -1.F
#define FLAP_SCREEN_RIGHT 1.F
#define FLAP_SCREEN_WIDTH 2.F

// Physics
#define FLAP_GRAVITY 2.F
#define FLAP_THRUST -0.75F
#define FLAP_THRUST_DELAY 0.1F
#define FLAP_SCROLL_SPEED -0.24F
#define FLAP_FALL_INITIAL_SPEED 0.1F

// Bird
#define FLAP_BIRD_X -0.75F
#define FLAP_BIRD_Y -0.5F
#define FLAP_BIRD_WIDTH 0.08F
#define FLAP_BIRD_HEIGHT 0.14F

// Pipes
#define FLAP_PIPE_WIDTH 0.12F
#define FLAP_PIPE_MIN_HEIGHT 0.25F
#define FLAP_PIPE_MAX_HEIGHT 1.F
#define FLAP_PIPE_INITIAL_GAP 0.64F
#define FLAP_PIPE_STEP 0.5F

/**
 * Initialize game resources.
 */
void game_init();

/**
 * Free resources.
 */
void game_quit();

/**
 * Reset game state.
 */
void game_reset();

/**
 * Update physics
 */
void game_update();

#endif // FLAP_GAME_H
