#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "flap.h"

#include "rect.h"
#include "window.h"

typedef enum { STATE_PLAYING, STATE_FALLING, STATE_GAMEOVER } GameState;

static GameState game_state = STATE_PLAYING;
static Rect *bird;
static Rect *pipes[FLAP_NUM_PIPES * 2];

static float speed_x = 0.0f;
static float speed_y = 0.0f;

static void init() {
  game_state = STATE_PLAYING;

  rect_set_position(bird, FLAP_BIRD_X, FLAP_BIRD_Y);

  for (int i = 0; i < FLAP_NUM_PIPES * 2; i += 2) {
    float x = i * FLAP_PIPE_STEP;
    float h = FLAP_PIPE_MIN_HEIGHT +
              ((float)rand()) / ((float)RAND_MAX) *
              (FLAP_PIPE_MAX_HEIGHT - FLAP_PIPE_MIN_HEIGHT);

    rect_set_position(pipes[i], x, -1.0f);
    rect_set_size(pipes[i], FLAP_PIPE_WIDTH, h);


    rect_set_position(pipes[i + 1], x, -1.0f + h + FLAP_PIPE_OPENING);
    rect_set_size(pipes[i + 1], FLAP_PIPE_WIDTH, 2.0f - h);
  }

  speed_x = 0.0f;
  speed_y = 0.0f;
}

int main(int argc, char **argv) {
  srand((unsigned int)time(NULL));

  window_init();
  rect_init();

  bird = rect_new();
  rect_set_size(bird, FLAP_BIRD_WIDTH, FLAP_BIRD_HEIGHT);

  for (int i = 0; i < FLAP_NUM_PIPES * 2; i += 2) {
    pipes[i] = rect_new();
    pipes[i + 1] = rect_new();
  }

  init();

  float start_time = window_get_time();

  int running = 1;
  while (!window_should_close()) {
    float now = window_get_time();
    float dt = now - start_time;
    start_time = now;

    window_update();

    if (game_state == STATE_PLAYING) {
      speed_y += (FLAP_THRUST * window_get_thrust() - FLAP_GRAVITY) * dt;

      for (int i = 0; i < FLAP_NUM_PIPES; i += 2) {
        rect_move(pipes[i], FLAP_SCROLL_SPEED * dt, 0);
        rect_move(pipes[i + 1], FLAP_SCROLL_SPEED * dt, 0);

        if (rect_get_x(pipes[i]) < -1.0f - FLAP_PIPE_WIDTH) {
          rect_move(pipes[i], 2.0f + FLAP_PIPE_WIDTH, 0);
          rect_move(pipes[i + 1], 2.0f + FLAP_PIPE_WIDTH, 0);
        } else if (rect_intersect(bird, pipes[i]) ||
                   rect_intersect(bird, pipes[i + 1])) {
          speed_x = FLAP_FALL_INITIAL_SPEED;
          speed_y = FLAP_FALL_INITIAL_SPEED;
          game_state = STATE_FALLING;
        }
      }
      if (rect_get_y(bird) < -1.0f) {
        game_state = STATE_GAMEOVER;
      }
    } else if (game_state == STATE_FALLING) {
      speed_y -= FLAP_GRAVITY * dt;
      if (rect_get_y(bird) < -1.0f) {
        game_state = STATE_GAMEOVER;
      }
    } else { // STATE_GAMEOVER
      if (window_get_thrust()) {
        init();
      }
    }

    rect_move(bird, speed_x * dt, speed_y * dt);

    rect_draw();

    window_render();
  }

  rect_quit();
  window_quit();

  return 0;
}
