#include "game.h"

#include <stdlib.h>

#include "flap.h"
#include "sprite.h"
#include "window.h"

typedef enum { STATE_PLAYING, STATE_FALLING, STATE_GAMEOVER } GameState;

static GameState game_state = STATE_PLAYING;

static int pause = 0;

static float speed_x = 0.F;
static float speed_y = 0.F;

static const float SECOND = 1000.F;
static float last_time = 0.F;
static float last_thrust = 0.F;

static Sprite *bird = NULL;

static Sprite *pipes[4 * FLAP_NUM_PIPES] = {NULL};
static float pipe_gap = FLAP_PIPE_INITIAL_GAP;

static const float pipe_body_width =
    (float)FLAP_SPRITE_TEXTURE_PIPE_BODY_WIDTH /
    (float)FLAP_SPRITE_TEXTURE_PIPE_HEAD_WIDTH * FLAP_PIPE_WIDTH;

static const float pipe_body_x =
    ((float)FLAP_PIPE_WIDTH - pipe_body_width) / 2.F;

void game_init() {
  bird = sprite_new(FLAP_SPRITE_TEXTURE_BIRD_X, FLAP_SPRITE_TEXTURE_BIRD_Y,
                    FLAP_SPRITE_TEXTURE_BIRD_WIDTH,
                    FLAP_SPRITE_TEXTURE_BIRD_HEIGHT);
  sprite_set_w(bird, FLAP_BIRD_WIDTH);
  sprite_set_h(bird, FLAP_BIRD_HEIGHT);

  for (int i = 0; i < 4 * FLAP_NUM_PIPES; i += 4) {
    // Top pipe body
    pipes[i] = sprite_new(FLAP_SPRITE_TEXTURE_PIPE_BODY_X,
                          FLAP_SPRITE_TEXTURE_PIPE_BODY_Y,
                          FLAP_SPRITE_TEXTURE_PIPE_BODY_WIDTH,
                          FLAP_SPRITE_TEXTURE_PIPE_BODY_HEIGHT);

    // Top pipe head
    pipes[i + 1] = sprite_new(FLAP_SPRITE_TEXTURE_PIPE_HEAD_X,
                              FLAP_SPRITE_TEXTURE_PIPE_HEAD_Y,
                              FLAP_SPRITE_TEXTURE_PIPE_HEAD_WIDTH,
                              FLAP_SPRITE_TEXTURE_PIPE_HEAD_HEIGHT);

    // Bottom pipe head
    pipes[i + 2] = sprite_new(FLAP_SPRITE_TEXTURE_PIPE_HEAD_X,
                              FLAP_SPRITE_TEXTURE_PIPE_HEAD_Y,
                              FLAP_SPRITE_TEXTURE_PIPE_HEAD_WIDTH,
                              FLAP_SPRITE_TEXTURE_PIPE_HEAD_HEIGHT);

    // Bottom pipe body
    pipes[i + 3] = sprite_new(FLAP_SPRITE_TEXTURE_PIPE_BODY_X,
                              FLAP_SPRITE_TEXTURE_PIPE_BODY_Y,
                              FLAP_SPRITE_TEXTURE_PIPE_BODY_WIDTH,
                              FLAP_SPRITE_TEXTURE_PIPE_BODY_HEIGHT);
  }

  game_reset();
}

void game_reset() {
  game_state = STATE_PLAYING;

  sprite_set_x(bird, FLAP_BIRD_X);
  sprite_set_y(bird, FLAP_BIRD_Y);

  for (int i = 0; i < 4 * FLAP_NUM_PIPES; i += 4) {
    float x = i / 4 * FLAP_PIPE_STEP;
    float h = FLAP_PIPE_MIN_HEIGHT +
              (float)rand() / (float)RAND_MAX *
                  (FLAP_PIPE_MAX_HEIGHT - FLAP_PIPE_MIN_HEIGHT);

    // Top pipe body

    sprite_set_x(pipes[i], x + pipe_body_x);
    sprite_set_y(pipes[i], FLAP_SCREEN_TOP);
    sprite_set_w(pipes[i], pipe_body_width);
    sprite_set_h(pipes[i], h);
    sprite_set_th(pipes[i], 2 * h / FLAP_PIPE_WIDTH);

    // Top pipe head
    sprite_set_x(pipes[i + 1], x);
    sprite_set_y(pipes[i + 1], FLAP_SCREEN_TOP + h);
    sprite_set_w(pipes[i + 1], FLAP_PIPE_WIDTH);
    sprite_set_h(pipes[i + 1], FLAP_SPRITE_PIPE_HEAD_HEIGHT * FLAP_PIPE_WIDTH);

    // Bottom pipe head
    sprite_set_x(pipes[i + 2], x);
    sprite_set_y(pipes[i + 2], FLAP_SCREEN_TOP + h + pipe_gap);
    sprite_set_w(pipes[i + 2], FLAP_PIPE_WIDTH);
    sprite_set_h(pipes[i + 2], FLAP_SPRITE_PIPE_HEAD_HEIGHT * FLAP_PIPE_WIDTH);

    // Bottom pipe body
    sprite_set_x(pipes[i + 3], x + pipe_body_x);
    sprite_set_y(pipes[i + 3],
                 FLAP_SCREEN_TOP + h + pipe_gap +
                     FLAP_SPRITE_PIPE_HEAD_HEIGHT * FLAP_PIPE_WIDTH);
    sprite_set_w(pipes[i + 3], pipe_body_width);
    sprite_set_h(pipes[i + 3],
                 FLAP_SCREEN_HEIGHT - h -
                     FLAP_SPRITE_PIPE_HEAD_HEIGHT * FLAP_PIPE_WIDTH);
    sprite_set_th(pipes[i + 3], 2 * h / FLAP_PIPE_WIDTH);
  }

  speed_x = 0.F;
  speed_y = 0.F;
}

void game_update() {
  float now = window_get_time();
  float dt = now - last_time;
  last_time = now;

  if (pause) {
    if (window_get_pause()) {
      pause = 0;
    }
    return;
  } else if (window_get_pause()) {
    pause = 1;
  }

  if (game_state == STATE_PLAYING) {
    speed_y += FLAP_GRAVITY * dt;

    if (window_get_thrust() && now - last_thrust > FLAP_THRUST_DELAY) {
      speed_y += FLAP_THRUST;
      last_thrust = now;
    }

    for (int i = 0; i < 4 * FLAP_NUM_PIPES; i += 4) {
      float new_x = sprite_get_x(pipes[i + 1]) + FLAP_SCROLL_SPEED * dt;

      sprite_set_x(pipes[i], new_x + pipe_body_x);
      sprite_set_x(pipes[i + 1], new_x);
      sprite_set_x(pipes[i + 2], new_x);
      sprite_set_x(pipes[i + 3], new_x + pipe_body_x);

      // Set pipes back to the far right
      if (sprite_get_x(pipes[i]) < FLAP_SCREEN_LEFT - FLAP_PIPE_WIDTH) {
        float far_away = FLAP_SCREEN_WIDTH;

        // Increase difficulty over time
        pipe_gap =
            FLAP_PIPE_INITIAL_GAP - (now / SECOND) * FLAP_PIPE_INITIAL_GAP;

        float new_height = FLAP_PIPE_MIN_HEIGHT +
                           (float)rand() / (float)RAND_MAX *
                               (FLAP_PIPE_MAX_HEIGHT - FLAP_PIPE_MIN_HEIGHT);

        // Top pipe body
        sprite_set_x(pipes[i], far_away);
        sprite_set_h(pipes[i], new_height);
        sprite_set_th(pipes[i], 2 * new_height / FLAP_PIPE_WIDTH);

        // Top pipe head
        sprite_set_x(pipes[i + 1], far_away);
        sprite_set_y(pipes[i + 1], FLAP_SCREEN_TOP + new_height);

        // Bottom pipe head
        sprite_set_x(pipes[i + 2], far_away);
        sprite_set_y(pipes[i + 2], FLAP_SCREEN_TOP + new_height + pipe_gap);

        // Bottom pipe body
        sprite_set_x(pipes[i + 3], far_away);
        sprite_set_y(pipes[i + 3],
                     FLAP_SCREEN_TOP + new_height + pipe_gap +
                         FLAP_SPRITE_PIPE_HEAD_HEIGHT * FLAP_PIPE_WIDTH);
        sprite_set_h(pipes[i + 3],
                     FLAP_SCREEN_HEIGHT - new_height -
                         FLAP_SPRITE_PIPE_HEAD_HEIGHT * FLAP_PIPE_WIDTH);
        sprite_set_th(pipes[i + 3], 2 * new_height / FLAP_PIPE_WIDTH);
      }
      // Collision detection
      else if (sprite_intersect(bird, pipes[i]) ||
               sprite_intersect(bird, pipes[i + 1]) ||
               sprite_intersect(bird, pipes[i + 2]) ||
               sprite_intersect(bird, pipes[i + 3]) ||
               sprite_get_y(bird) < FLAP_SCREEN_TOP) {
        game_state = STATE_FALLING;
        speed_x = -FLAP_FALL_INITIAL_SPEED;
        speed_y = FLAP_FALL_INITIAL_SPEED;
      }
    }
    if (sprite_get_y(bird) < FLAP_SCREEN_TOP) {
      game_state = STATE_FALLING;
      speed_x = -FLAP_FALL_INITIAL_SPEED;
      speed_y = FLAP_FALL_INITIAL_SPEED;
    } else if (sprite_get_y(bird) > FLAP_SCREEN_BOTTOM) {
      game_state = STATE_GAMEOVER;
    }
  } else if (game_state == STATE_FALLING) {
    speed_y += FLAP_GRAVITY * dt;
    if (sprite_get_y(bird) > FLAP_SCREEN_BOTTOM) {
      game_state = STATE_GAMEOVER;
    }
  } else { // STATE_GAMEOVER
    if (window_get_thrust()) {
      game_reset();
    }
  }

  sprite_set_x(bird, sprite_get_x(bird) + speed_x * dt);
  sprite_set_y(bird, sprite_get_y(bird) + speed_y * dt);
}
