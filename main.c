#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <GL/glew.h>
#include <graphene.h>

#include "flap.h"
#include "rect.h"
#include "window.h"

const float FLAP_BIRD_COLOR[] = {1.f, 1.f, 0.f};
const float FLAP_PIPE_COLOR[] = {0.035f, 0.42f, 0.035f};

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

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(flap_message_callback, 0);

  flap_GameState game_state = STATE_PLAYING;

  flap_rect_init();

  flap_Rect *bird = flap_rect_new();

  const float bird_pos[] = {FLAP_BIRD_X, FLAP_BIRD_Y};
  const float bird_size[] = {FLAP_BIRD_WIDTH, FLAP_BIRD_HEIGHT};

  graphene_vec2_init_from_float(&bird->pos, bird_pos);
  graphene_vec2_init_from_float(&bird->size, bird_size);
  graphene_vec3_init_from_float(&bird->color, FLAP_BIRD_COLOR);

  flap_Rect *pipes[8];
  for (int i = 0; i < 8; i += 2) {
    float x = -1.0 + i * FLAP_PIPE_STEP;
    float h = FLAP_PIPE_MIN_HEIGHT +
              ((float)rand()) / ((float)RAND_MAX) *
                  (FLAP_PIPE_MAX_HEIGHT - FLAP_PIPE_MIN_HEIGHT);

    pipes[i] = flap_rect_new();

    float pipe_pos[] = {x, -1.0f};
    float pipe_size[] = {FLAP_PIPE_WIDTH, h};

    graphene_vec2_init_from_float(&pipes[i]->pos, pipe_pos);
    graphene_vec2_init_from_float(&pipes[i]->size, pipe_size);
    graphene_vec3_init_from_float(&pipes[i]->color, FLAP_PIPE_COLOR);

    pipes[i + 1] = flap_rect_new();
    pipe_pos[0] = x;
    pipe_pos[1] = -1.0f + h + FLAP_PIPE_OPENING;
    pipe_size[1] = 2.0f - h - 0.32f;

    graphene_vec2_init_from_float(&pipes[i + 1]->pos, pipe_pos);
    graphene_vec2_init_from_float(&pipes[i + 1]->size, pipe_size);
    graphene_vec3_init_from_float(&pipes[i + 1]->color, FLAP_PIPE_COLOR);
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

      float movement[2] = {0};
      movement[1] = accel_y;
      graphene_vec2_t dy;

      graphene_vec2_init_from_float(&dy, movement);
      graphene_vec2_add(&bird->pos, &dy, &bird->pos);

      if (graphene_vec2_get_y(&bird->pos) < -1.0f) {
        game_state = STATE_GAMEOVER;
      }

      for (int i = 0; i < 4; i += 2) {
        float pipe_movement[2] = {0};
        pipe_movement[0] = -0.004f;

        graphene_vec2_t scroll;

        graphene_vec2_init_from_float(&scroll, pipe_movement);

        graphene_vec2_add(&pipes[i]->pos, &scroll, &pipes[i]->pos);
        graphene_vec2_add(&pipes[i + 1]->pos, &scroll, &pipes[i + 1]->pos);

		float pipe_x = graphene_vec2_get_x(&pipes[i]->pos),
			  pipe_y = graphene_vec2_get_y(&pipes[i]->pos),
    		  pipe_h = graphene_vec2_get_y(&pipes[i]->size),
              pipe_upper_y = graphene_vec2_get_y(&pipes[i + 1]->pos),
              bird_y = graphene_vec2_get_y(&bird->pos),
              bird_h = graphene_vec2_get_y(&bird->size);

        if (pipe_x < -1.16f) {
          graphene_vec2_t set_pipe_back;
          pipe_movement[0] = 2.0f + FLAP_PIPE_WIDTH;
          graphene_vec2_init_from_float(&set_pipe_back, pipe_movement);

          graphene_vec2_add(&pipes[i]->pos, &set_pipe_back, &pipes[i]->pos);
          graphene_vec2_add(&pipes[i + 1]->pos, &set_pipe_back,
                            &pipes[i + 1]->pos);
        } else if (pipe_x > -0.75f - 0.08f && pipe_x < -0.75f + 0.08f &&
                   (bird_y < pipe_y + pipe_h ||
                    bird_y > pipe_upper_y - bird_h)) {
          game_state = STATE_GAMEOVER;
        }
      }

    } else if (game_state == STATE_GAMEOVER) {
    }

    // Blue sky
    glClearColor(0.53f, 0.81f, 0.92f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    flap_rect_draw();

    flap_window_render(window);
  }

  flap_rect_quit();

  flap_window_free(window);

  return 0;
}
