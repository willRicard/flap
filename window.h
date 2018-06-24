#ifndef FLAP_WINDOW_H_
#define FLAP_WINDOW_H_

typedef struct flap_Window flap_Window;

flap_Window *flap_window_new();

void flap_window_free(flap_Window *window);

int flap_window_should_close(flap_Window *window);

int flap_window_thrust(flap_Window *window);

void flap_window_update(flap_Window *window);

void flap_window_render(flap_Window *window);

#endif // FLAP_WINDOW_H_
