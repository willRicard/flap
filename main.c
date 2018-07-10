#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <GL/glew.h>

#include "flap.h"
#include "rect.h"
#include "window.h"

typedef enum { STATE_PLAYING, STATE_GAMEOVER } flap_GameState;

void GLAPIENTRY flap_message_callback(GLenum source, GLenum type, GLuint id,
                                      GLenum severity, GLsizei length,
                                      const GLchar *message,
                                      const void *param) {

  fprintf(stderr, "OpenGL: %s\n", message);
}

int main(int argc, char **argv) {
  srand((unsigned int)time(NULL));

  flap_Window *window = flap_window_new();

  //glEnable(GL_DEBUG_OUTPUT);
  //glDebugMessageCallback(flap_message_callback, 0);

  flap_GameState game_state = STATE_PLAYING;

  flap_rect_init();

  flap_Rect *bird = flap_rect_new();

  flap_rect_set_position(bird, FLAP_BIRD_X, FLAP_BIRD_Y);
  flap_rect_set_size(bird, FLAP_BIRD_WIDTH, FLAP_BIRD_HEIGHT);

  flap_Rect *pipes[8];
  for (int i = 0; i < FLAP_NUM_PIPES; i += 2) {
    float x = -1.0 + i * FLAP_PIPE_STEP;
    float h = FLAP_PIPE_MIN_HEIGHT +
              ((float)rand()) / ((float)RAND_MAX) *
                  (FLAP_PIPE_MAX_HEIGHT - FLAP_PIPE_MIN_HEIGHT);

    pipes[i] = flap_rect_new();

    flap_rect_set_position(pipes[i], x, -1.0f);
    flap_rect_set_size(pipes[i], FLAP_PIPE_WIDTH, h);

    pipes[i + 1] = flap_rect_new();
    flap_rect_set_position(pipes[i + 1], x, -1.0f + h + FLAP_PIPE_OPENING);
    flap_rect_set_size(pipes[i + 1], FLAP_PIPE_WIDTH, 2.0f - h - 0.32f);
  }

  float accel_y = 0.0f;

  int running = 1;
  while (running) {
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

        float pipe_x = flap_rect_get_x(pipes[i]),
              pipe_y = flap_rect_get_y(pipes[i]),
              pipe_h = flap_rect_get_width(pipes[i]),
              pipe_upper_y = flap_rect_get_y(pipes[i + 1]),
              bird_y = flap_rect_get_y(bird),
              bird_h = flap_rect_get_height(bird);

        if (pipe_x < -1.16f) {
          flap_rect_move(pipes[i], 2.0f + FLAP_PIPE_WIDTH, 0);
          flap_rect_move(pipes[i + 1], 2.0f + FLAP_PIPE_WIDTH, 0);
        } else if (pipe_x > -0.75f - 0.08f && pipe_x < -0.75f + 0.08f &&
                   (bird_y < pipe_y + pipe_h ||
                    bird_y > pipe_upper_y - bird_h)) {
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
