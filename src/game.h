#ifndef FLAP_GAME_H
#define FLAP_GAME_H

/**
 * Initialize game resources.
 */
void game_init();

/**
 * Free resources.
 */
void game_quit();

/**
 * Reset game state.
 */
void game_reset();

/**
 * Update physics
 */
void game_update();

#endif // FLAP_GAME_H
