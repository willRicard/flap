#include "game.h"

#include <stdio.h>
#include <stdlib.h>

#include "flap.h"
#include "sprite.h"
#include "window.h"

typedef enum { STATE_PLAYING, STATE_FALLING, STATE_GAMEOVER } GameState;

static GameState game_state;
static Sprite *bird;

static float vx = 0.0f, vy = 0.0f;

static float start_time = 0.0f, last_time = 0.0f, last_thrust = 0.0f;

static Sprite *pipes[FLAP_NUM_PIPES * 2];
static float pipe_gap = FLAP_PIPE_INITIAL_GAP;

void game_init() {
  bird = sprite_new(0, 0, 32, 32);
  sprite_set_w(bird, FLAP_BIRD_WIDTH);
  sprite_set_h(bird, FLAP_BIRD_HEIGHT);

  for (int i = 0; i < FLAP_NUM_PIPES * 2; i += 2) {
    pipes[i] = sprite_new(64, 32, 32, -32);
    pipes[i + 1] = sprite_new(64, 0, 32, 32);
  }

  game_reset();
}

void game_reset() {
  game_state = STATE_PLAYING;
  start_time = window_get_time();

  sprite_set_x(bird, FLAP_BIRD_X);
  sprite_set_y(bird, FLAP_BIRD_Y);

  for (int i = 0; i < FLAP_NUM_PIPES * 2; i += 2) {
    float x = i * FLAP_PIPE_STEP;
    float h = FLAP_PIPE_MIN_HEIGHT +
              (float)rand() / (float)RAND_MAX *
                  (FLAP_PIPE_MAX_HEIGHT - FLAP_PIPE_MIN_HEIGHT);

    // Top pipe
    sprite_set_x(pipes[i], x);
    sprite_set_y(pipes[i], -1.0f);
    sprite_set_w(pipes[i], FLAP_PIPE_WIDTH);
    sprite_set_h(pipes[i], h);

    // Bottom pipe
    sprite_set_x(pipes[i + 1], x);
    sprite_set_y(pipes[i + 1], -1.0f + h + pipe_gap);
    sprite_set_w(pipes[i + 1], FLAP_PIPE_WIDTH);
    sprite_set_h(pipes[i + 1], 2.0f - h);
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
      sprite_set_x(pipes[i], sprite_get_x(pipes[i]) + FLAP_SCROLL_SPEED * dt);
      sprite_set_x(pipes[i + 1],
                   sprite_get_x(pipes[i + 1]) + FLAP_SCROLL_SPEED * dt);

      // Set pipes back to the far right
      if (sprite_get_x(pipes[i]) < -1.0f - FLAP_PIPE_WIDTH) {
        pipe_gap =
            FLAP_PIPE_INITIAL_GAP - (now / 1000.f) * FLAP_PIPE_INITIAL_GAP;
        float h = FLAP_PIPE_MIN_HEIGHT +
                  (float)rand() / (float)RAND_MAX *
                      (FLAP_PIPE_MAX_HEIGHT - FLAP_PIPE_MIN_HEIGHT);
        sprite_set_x(pipes[i], sprite_get_x(pipes[i]) + 2.0f + FLAP_PIPE_WIDTH);
        sprite_set_h(pipes[i], h);

        sprite_set_x(pipes[i + 1],
                     sprite_get_x(pipes[i + 1]) + 2.0f + FLAP_PIPE_WIDTH);
        sprite_set_y(pipes[i + 1], -1.0f + h + pipe_gap);
        sprite_set_h(pipes[i + 1], 2.0f - h);
      }
      // Collision detection
      else if (sprite_intersect(bird, pipes[i]) ||
               sprite_intersect(bird, pipes[i + 1]) ||
               sprite_get_y(bird) < -1.0) {
        game_state = STATE_FALLING;
        vx = -FLAP_FALL_INITIAL_SPEED;
        vy = FLAP_FALL_INITIAL_SPEED;
      }
    }
    if (sprite_get_y(bird) > 1.0f) {
      game_state = STATE_GAMEOVER;
    }
  } else if (game_state == STATE_FALLING) {
    vy += FLAP_GRAVITY * dt;
    if (sprite_get_y(bird) > 1.0f) {
      game_state = STATE_GAMEOVER;
    }
  } else { // STATE_GAMEOVER
    if (window_get_thrust()) {
      game_reset();
    }
  }

  sprite_set_x(bird, sprite_get_x(bird) + vx * dt);
  sprite_set_y(bird, sprite_get_y(bird) + vy * dt);
}
