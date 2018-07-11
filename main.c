#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "flap.h"
#include "rect.h"
#include "window.h"

typedef enum { STATE_PLAYING, STATE_GAMEOVER } flap_GameState;

int main(int argc, char **argv) {
  srand((unsigned int)time(NULL));

  flap_Window *window = flap_window_new();

  flap_GameState game_state = STATE_PLAYING;

  flap_rect_init();

  flap_Rect *bird = flap_rect_new();

  flap_rect_set_position(bird, FLAP_BIRD_X, FLAP_BIRD_Y);
  flap_rect_set_size(bird, FLAP_BIRD_WIDTH, FLAP_BIRD_HEIGHT);

  flap_Rect *pipes[8];
  for (int i = 0; i < FLAP_NUM_PIPES; i += 2) {
    float x = i * FLAP_PIPE_STEP;
    float h = FLAP_PIPE_MIN_HEIGHT +
              ((float)rand()) / ((float)RAND_MAX) *
                  (FLAP_PIPE_MAX_HEIGHT - FLAP_PIPE_MIN_HEIGHT);

    pipes[i] = flap_rect_new();

    flap_rect_set_position(pipes[i], x, -1.0f);
    flap_rect_set_size(pipes[i], FLAP_PIPE_WIDTH, h);

    pipes[i + 1] = flap_rect_new();
    flap_rect_set_position(pipes[i + 1], x, -1.0f + h + FLAP_PIPE_OPENING);
    flap_rect_set_size(pipes[i + 1], FLAP_PIPE_WIDTH, 2.0f - h);
  }

  float accel_y = 0.0f;

  unsigned long num_frames = 0;

  clock_t start_time = clock();

  int running = 1;
  while (running) {
    num_frames++;
    clock_t now = clock();
    if (now - start_time > 1000000) {
      printf("OK\n");
      printf("Current ms/frame: %f\n", 1000.0 / (double)(num_frames));
      num_frames = 0;
      start_time = now;
    }

    running = !flap_window_should_close(window);

    flap_window_update(window);

    if (game_state == STATE_PLAYING) {
      if (flap_window_thrust(window) && accel_y < 0.02f) {
        accel_y += 0.004f;
      } else {
        accel_y -= 0.001f;
      }

      flap_rect_move(bird, 0, accel_y);

      if (flap_rect_get_y(bird) < -1.0f) {
        game_state = STATE_GAMEOVER;
      }

      for (int i = 0; i < FLAP_NUM_PIPES; i += 2) {
        flap_rect_move(pipes[i], FLAP_SCROLL_SPEED, 0);
        flap_rect_move(pipes[i + 1], FLAP_SCROLL_SPEED, 0);

        if (flap_rect_get_x(pipes[i]) < -1.0f - FLAP_PIPE_WIDTH) {
          flap_rect_move(pipes[i], 2.0f + FLAP_PIPE_WIDTH, 0);
          flap_rect_move(pipes[i + 1], 2.0f + FLAP_PIPE_WIDTH, 0);
          // Collision test: pipe_x > FLAP_BIRD_X && pipe_x < FLAP_BIRD_X +
          // FLAP_BIRD_WIDTH &&
        } else if (flap_rect_intersect(bird, pipes[i]) ||
                   flap_rect_intersect(bird, pipes[i + 1])) {
          game_state = STATE_GAMEOVER;
        }
      }

    } else if (game_state == STATE_GAMEOVER) {
    }

    flap_rect_draw();

    flap_window_render(window);
  }

  flap_rect_quit();

  flap_window_free(window);

  return 0;
}
