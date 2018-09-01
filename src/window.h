#ifndef FLAP_WINDOW_H_
#define FLAP_WINDOW_H_

/*
 * A window for rendering.
 */

void window_init();

void window_quit();

int window_should_close();

float window_get_time();

int window_get_thrust();

void window_update();

void window_render();

void window_fail_with_error(const char *error);

#endif // FLAP_WINDOW_H_
