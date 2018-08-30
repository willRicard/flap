#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "flap.h"

#include "rect.h"
#include "window.h"

typedef enum { STATE_PLAYING, STATE_FALLING, STATE_GAMEOVER } flapGameState;

int main(int argc, char **argv) {
  srand((unsigned int)time(NULL));

  flapWindowInit();
  flapRectInit();

  flapGameState gameState = STATE_PLAYING;

  flapRect *bird = flapRectNew();

  flapRectSetPosition(bird, FLAP_BIRD_X, FLAP_BIRD_Y);
  flapRectSetSize(bird, FLAP_BIRD_WIDTH, FLAP_BIRD_HEIGHT);

  flapRect *pipes[8];
  for (int i = 0; i < FLAP_NUM_PIPES * 2; i += 2) {
    float x = i * FLAP_PIPE_STEP;
    float h = FLAP_PIPE_MIN_HEIGHT +
              ((float)rand()) / ((float)RAND_MAX) *
                  (FLAP_PIPE_MAX_HEIGHT - FLAP_PIPE_MIN_HEIGHT);

    pipes[i] = flapRectNew();

    flapRectSetPosition(pipes[i], x, -1.0f);
    flapRectSetSize(pipes[i], FLAP_PIPE_WIDTH, h);

    pipes[i + 1] = flapRectNew();
    flapRectSetPosition(pipes[i + 1], x, -1.0f + h + FLAP_PIPE_OPENING);
    flapRectSetSize(pipes[i + 1], FLAP_PIPE_WIDTH, 2.0f - h);
  }

  float speedX = 0.0f;
  float speedY = 0.0f;

  float startTime = flapWindowGetTime();

  int running = 1;
  while (!flapWindowShouldClose()) {
    float now = flapWindowGetTime();
    float dt = now - startTime;
    startTime = now;

    flapWindowUpdate();

    if (gameState == STATE_PLAYING) {
      speedY += (FLAP_THRUST * flapWindowGetThrust() - FLAP_GRAVITY) * dt;

      for (int i = 0; i < FLAP_NUM_PIPES; i += 2) {
        float scrollDeltaX = FLAP_SCROLL_SPEED * dt;
        flapRectMove(pipes[i], scrollDeltaX, 0);
        flapRectMove(pipes[i + 1], scrollDeltaX, 0);

        if (flapRectGetX(pipes[i]) < -1.0f - FLAP_PIPE_WIDTH) {
          flapRectMove(pipes[i], 2.0f + FLAP_PIPE_WIDTH, 0);
          flapRectMove(pipes[i + 1], 2.0f + FLAP_PIPE_WIDTH, 0);
        } else if (flapRectIntersect(bird, pipes[i]) ||
                   flapRectIntersect(bird, pipes[i + 1])) {
          speedX = FLAP_FALL_INITIAL_SPEED;
          speedY = FLAP_FALL_INITIAL_SPEED;
          gameState = STATE_FALLING;
        }
      }
      if (flapRectGetY(bird) < -1.0f) {
        gameState = STATE_GAMEOVER;
      }
    } else if (gameState == STATE_FALLING) {
      speedY -= FLAP_GRAVITY * dt;
      if (flapRectGetY(bird) < -1.0f) {
        gameState = STATE_GAMEOVER;
      }
    }

    flapRectMove(bird, speedX * dt, speedY * dt);

    flapRectDraw();

    flapWindowRender();
  }

  flapRectQuit();
  flapWindowQuit();

  return 0;
}
