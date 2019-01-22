#include "game.h"

#include <stdlib.h>

#include "flap.h"
#include "rect.h"
#include "window.h"

typedef enum { STATE_PLAYING, STATE_FALLING, STATE_GAMEOVER } GameState;

static GameState game_state;
static Rect *bird;

static float vx = 0.0f, vy = 0.0f;

static float start_time = 0.0f, last_time = 0.0f, last_thrust = 0.0f;

static Rect *pipes[FLAP_NUM_PIPES * 2];
static float pipe_gap = FLAP_PIPE_INITIAL_GAP;

void game_init() {
  bird = rect_new();
  bird->w = FLAP_BIRD_WIDTH;
  bird->h = FLAP_BIRD_HEIGHT;

  for (int i = 0; i < FLAP_NUM_PIPES * 2; i += 2) {
    pipes[i] = rect_new();
    pipes[i + 1] = rect_new();
  }

  game_reset();
}

void game_reset() {
  game_state = STATE_PLAYING;
  start_time = window_get_time();

  bird->x = FLAP_BIRD_X;
  bird->y = FLAP_BIRD_Y;

  for (int i = 0; i < FLAP_NUM_PIPES * 2; i += 2) {
    float x = i * FLAP_PIPE_STEP;
    float h = FLAP_PIPE_MIN_HEIGHT +
              (float)rand() / (float)RAND_MAX *
                  (FLAP_PIPE_MAX_HEIGHT - FLAP_PIPE_MIN_HEIGHT);

    // Top pipe
    pipes[i]->x = x;
    pipes[i]->y = -1.0f;
    pipes[i]->w = FLAP_PIPE_WIDTH;
    pipes[i]->h = h;

    // Bottom pipe
    pipes[i + 1]->x = x;
    pipes[i + 1]->y = -1.0f + h + pipe_gap;
    pipes[i + 1]->w = FLAP_PIPE_WIDTH;
    pipes[i + 1]->h = 2.0f - h;
  }

  vx = 0.0f;
  vy = 0.0f;
}

void game_update() {
  float now = window_get_time();
  float dt = now - last_time;
  last_time = now;

  if (game_state == STATE_PLAYING) {
    vy += FLAP_GRAVITY * dt;

    if (window_get_thrust() && now - last_thrust > 0.1f) {
      vy += FLAP_THRUST;
      last_thrust = now;
    }

    for (int i = 0; i < FLAP_NUM_PIPES; i += 2) {
      pipes[i]->x += FLAP_SCROLL_SPEED * dt;
      pipes[i + 1]->x += FLAP_SCROLL_SPEED * dt;

      // Set pipes back to the far right
      pipe_gap = FLAP_PIPE_INITIAL_GAP - (now / 1000.f) * FLAP_PIPE_INITIAL_GAP;
      if (pipes[i]->x < -1.0f - FLAP_PIPE_WIDTH) {
        float h = FLAP_PIPE_MIN_HEIGHT +
                  (float)rand() / (float)RAND_MAX *
                      (FLAP_PIPE_MAX_HEIGHT - FLAP_PIPE_MIN_HEIGHT);
        pipes[i]->x += 2.0f + FLAP_PIPE_WIDTH;
        pipes[i]->h = h;

        pipes[i + 1]->x += 2.0f + FLAP_PIPE_WIDTH;
        pipes[i + 1]->y = -1.0f + h + pipe_gap;
        pipes[i + 1]->h = 2.0f - h;
      }
      // Collision detection
      else if (rect_intersect(bird, pipes[i]) ||
               rect_intersect(bird, pipes[i + 1]) || bird->y < -1.0) {
        game_state = STATE_FALLING;
        vx = -FLAP_FALL_INITIAL_SPEED;
        vy = FLAP_FALL_INITIAL_SPEED;
      }
    }
    if (bird->y > 1.0f) {
      game_state = STATE_GAMEOVER;
    }
  } else if (game_state == STATE_FALLING) {
    vy += FLAP_GRAVITY * dt;
    if (bird->y > 1.0f) {
      game_state = STATE_GAMEOVER;
    }
  } else { // STATE_GAMEOVER
    if (window_get_thrust()) {
      game_reset();
    }
  }

  bird->x += vx * dt;
  bird->y += vy * dt;
}
