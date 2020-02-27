#include "game.h"

#include <stddef.h>
#include <time.h>

#include "xoroshiro.h"

#include "sprite.h"
#include "window.h"

// Vulkan coordinate system.
static const float kScreenTop = -1.F;
static const float kScreenBottom = 1.F;
static const float kScreenHeight = 2.F;
static const float kScreenLeft = -1.F;
static const float kScreenRight = 1.F;
static const float kScreenWidth = 2.F;

// Physics
static const float kGravity = 2.F;
static const float kThrust = -0.75F;
static const float kThrustDelay = 0.1F;
static const float kScrollSpeed = -0.24F;
static const float kFallSpeed = 0.1F;

// Increase difficulty over time:
// Game can't last more than 2 minutes.
static const float kDeadline = 120.F;

// Bird
static const float kBirdX = -0.75F;
static const float kBirdY = -0.5F;
static const float kBirdWidth = 0.08F;
static const float kBirdHeight = 0.14F;
static const float kBirdTextureX = 0.F;
static const float kBirdTextureY = 0.F;
static const float kBirdTextureWidth = 32.F;
static const float kBirdTextureHeight = 32.F;

// Pipes
static const float kPipeWidth = 0.12F;
static const float kMinPipeHeight = 0.25F;
static const float kMaxPipeHeight = 1.F;
static const float kInitialPipeGap = 0.64F;
static const float kPipeStep = 0.5F;

static const float kPipeHeadHeight = 16.F / 32.F;
static const float kPipeHeadTextureX = 64.F;
static const float kPipeHeadTextureY = 0.F;
static const float kPipeHeadTextureWidth = 32.F;
static const float kPipeHeadTextureHeight = 16.F;

static const float kPipeBodyTextureX = 102.F;
static const float kPipeBodyTextureY = 0.F;
static const float kPipeBodyTextureWidth = 20.F;
static const float kPipeBodyTextureHeight = 32.F;

// kPipeBodyWidth = kPipeBodyTextureWidth / kPipeHeadTextureWidth * kPipeWidth;
static const float kPipeBodyWidth = 0.075F;

// kPipeBodyX = (kPipeWidth - kPipeBodyWidth) / 2.F;
static const float kPipeBodyX = 0.0225F;

static uint64_t random_generator_state[2] = {0};

typedef enum { STATE_PLAYING, STATE_FALLING, STATE_GAMEOVER } GameState;

static GameState game_state = STATE_PLAYING;

static int pause = 0;

static float speed_x = 0.F;
static float speed_y = 0.F;

static float last_time = 0.F;
static float last_thrust = 0.F;

static Sprite *bird = NULL;

static Sprite *pipes[kNumSprites] = {NULL};
static int next_pipe = 0;
static float pipe_gap = 0.F;

/**
 * Reset game state.
 */
static void game_reset() {
  game_state = STATE_PLAYING;

  pipe_gap = kInitialPipeGap;

  sprite_set_x(bird, kBirdX);
  sprite_set_y(bird, kBirdY);

  for (int i = 0; i < kSpritesPerPipe * kNumPipes; i += 4) {
    xoroshiro128plus(random_generator_state);

    const float x = (i / kSpritesPerPipe) * kPipeStep;
    const float h = kMinPipeHeight + (float)random_generator_state[0] /
                                         UINT64_MAX *
                                         (kMaxPipeHeight - kMinPipeHeight);

    // Top pipe body
    sprite_set_x(pipes[i], x + kPipeBodyX);
    sprite_set_y(pipes[i], kScreenTop);
    sprite_set_w(pipes[i], kPipeBodyWidth);
    sprite_set_h(pipes[i], h);
    sprite_set_th(pipes[i], 2 * h / kPipeWidth);

    // Top pipe head
    sprite_set_x(pipes[i + 1], x);
    sprite_set_y(pipes[i + 1], kScreenTop + h);
    sprite_set_w(pipes[i + 1], kPipeWidth);
    sprite_set_h(pipes[i + 1], kPipeHeadHeight * kPipeWidth);

    // Bottom pipe head
    sprite_set_x(pipes[i + 2], x);
    sprite_set_y(pipes[i + 2], kScreenTop + h + pipe_gap);
    sprite_set_w(pipes[i + 2], kPipeWidth);
    sprite_set_h(pipes[i + 2], kPipeHeadHeight * kPipeWidth);

    // Bottom pipe body
    sprite_set_x(pipes[i + 3], x + kPipeBodyX);
    sprite_set_y(pipes[i + 3],
                 kScreenTop + h + pipe_gap + kPipeHeadHeight * kPipeWidth);
    sprite_set_w(pipes[i + 3], kPipeBodyWidth);
    sprite_set_h(pipes[i + 3],
                 kScreenHeight - h - kPipeHeadHeight * kPipeWidth);
    sprite_set_th(pipes[i + 3], 2 * h / kPipeWidth);
  }
  next_pipe = 0;

  speed_x = 0.F;
  speed_y = 0.F;
}

/**
 * Initialize game resources.
 */
void game_init() {
  random_generator_state[0] = time(NULL) | 0xffe792a1 << 31;
  random_generator_state[1] = time(NULL) | 0xbffae98d << 31;

  bird = sprite_new(kBirdTextureX, kBirdTextureY, kBirdTextureWidth,
                    kBirdTextureHeight);
  sprite_set_w(bird, kBirdWidth);
  sprite_set_h(bird, kBirdHeight);

  for (int i = 0; i < kSpritesPerPipe * kNumPipes; i += 4) {
    // Top pipe body
    pipes[i] = sprite_new(kPipeBodyTextureX, kPipeBodyTextureY,
                          kPipeBodyTextureWidth, kPipeBodyTextureHeight);

    // Top pipe head
    pipes[i + 1] = sprite_new(kPipeHeadTextureX, kPipeHeadTextureY,
                              kPipeHeadTextureWidth, kPipeHeadTextureHeight);

    // Bottom pipe head
    pipes[i + 2] = sprite_new(kPipeHeadTextureX, kPipeHeadTextureY,
                              kPipeHeadTextureWidth, kPipeHeadTextureHeight);

    // Bottom pipe body
    pipes[i + 3] = sprite_new(kPipeBodyTextureX, kPipeBodyTextureY,
                              kPipeBodyTextureWidth, kPipeBodyTextureHeight);
  }

  game_reset();
}

static void scroll_pipes(const float dt) {
  for (int i = 0; i < kSpritesPerPipe * kNumPipes; i += 4) {
    const float new_x = sprite_get_x(pipes[i + 1]) + kScrollSpeed * dt;

    sprite_set_x(pipes[i], new_x + kPipeBodyX);
    sprite_set_x(pipes[i + 1], new_x);
    sprite_set_x(pipes[i + 2], new_x);
    sprite_set_x(pipes[i + 3], new_x + kPipeBodyX);
  }
}

/**
 * Update physics.
 */
void game_update() {
  const float now = window_get_time();
  const float dt = now - last_time;
  last_time = now;

  if (pause) {
    if (window_get_pause()) {
      pause = 0;
    }
    return;
  } else if (window_get_pause()) {
    pause = 1;
  }

  switch (game_state) {
  case STATE_PLAYING:
    speed_y += kGravity * dt;

    if (window_get_thrust() && now - last_thrust > kThrustDelay) {
      speed_y += kThrust;
      last_thrust = now;
    }

    scroll_pipes(dt);

    // Set pipes back to the far right
    if (sprite_get_x(pipes[next_pipe]) < kScreenLeft - kPipeWidth) {
      float far_away = kScreenRight;

      pipe_gap = kInitialPipeGap - (now / kDeadline) * kInitialPipeGap;

      xoroshiro128plus(random_generator_state);
      float new_height = kMinPipeHeight + (float)random_generator_state[0] /
                                              UINT64_MAX *
                                              (kMaxPipeHeight - kMinPipeHeight);

      // Top pipe body
      sprite_set_x(pipes[next_pipe], far_away);
      sprite_set_h(pipes[next_pipe], new_height);
      sprite_set_th(pipes[next_pipe], 2 * new_height / kPipeWidth);

      // Top pipe head
      sprite_set_x(pipes[next_pipe + 1], far_away);
      sprite_set_y(pipes[next_pipe + 1], kScreenTop + new_height);

      // Bottom pipe head
      sprite_set_x(pipes[next_pipe + 2], far_away);
      sprite_set_y(pipes[next_pipe + 2], kScreenTop + new_height + pipe_gap);

      // Bottom pipe body
      sprite_set_x(pipes[next_pipe + 3], far_away);
      sprite_set_y(pipes[next_pipe + 3], kScreenTop + new_height + pipe_gap +
                                             kPipeHeadHeight * kPipeWidth);
      sprite_set_h(pipes[next_pipe + 3],
                   kScreenHeight - new_height - kPipeHeadHeight * kPipeWidth);
      sprite_set_th(pipes[next_pipe + 3], 2 * new_height / kPipeWidth);

      next_pipe = (next_pipe + kSpritesPerPipe) % (kSpritesPerPipe * kNumPipes);
    }

    if (sprite_intersect(bird, pipes[next_pipe]) ||
        sprite_intersect(bird, pipes[next_pipe + 1]) ||
        sprite_intersect(bird, pipes[next_pipe + 2]) ||
        sprite_intersect(bird, pipes[next_pipe + 3]) ||
        sprite_get_y(bird) < kScreenTop) {
      game_state = STATE_FALLING;
      speed_x = -kFallSpeed;
      speed_y = kFallSpeed;
    } else if (sprite_get_y(bird) > kScreenBottom) {
      game_state = STATE_GAMEOVER;
    }
    break;
  case STATE_FALLING:
    speed_y += kGravity * dt;
    if (sprite_get_y(bird) > kScreenBottom) {
      game_state = STATE_GAMEOVER;
    }
    break;
  case STATE_GAMEOVER:
    if (window_get_thrust()) {
      game_reset();
    }
    break;
  default:
    break;
  }

  sprite_set_x(bird, sprite_get_x(bird) + speed_x * dt);
  sprite_set_y(bird, sprite_get_y(bird) + speed_y * dt);
}
