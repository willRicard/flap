#include <3ds.h>
#include <citro3d.h>

#include "game.h"
#include "sprite.h"
#include "window.h"

int main(void) {
  window_fail_with_error("Hello, world!");

  window_init();
  consoleInit(GFX_BOTTOM, NULL);

  sprite_init();

  game_init();

  window_update();

  while (!window_should_close()) {
    game_update();

    sprite_update();

    window_update();
    C3D_FrameEnd(0);
  }

  sprite_quit();

  window_quit();

  return 0;
}
