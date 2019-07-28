#pragma once

/**
 * A window for rendering.
 */
void window_init();

void window_quit();

void window_update();

void window_fail_with_error(const char *message);

int window_should_close();

float window_get_time();

int window_get_thrust();

int window_get_pause();
