#ifndef FLAP_RECT_H_
#define FLAP_RECT_H_

typedef struct flap_Rect {
  float vertices[12];
} flap_Rect;

void flap_rect_init();

void flap_rect_quit();

flap_Rect *flap_rect_new();


float flap_rect_get_x(flap_Rect *rect);

float flap_rect_get_y(flap_Rect *rect);

float flap_rect_get_width(flap_Rect *rect);

float flap_rect_get_height(flap_Rect *rect);


void flap_rect_set_x(flap_Rect *rect, float x);

void flap_rect_set_y(flap_Rect *rect, float y);

void flap_rect_set_position(flap_Rect *rect, float x, float y);

void flap_rect_move(flap_Rect *rect, float x, float y);

void flap_rect_set_width(flap_Rect *rect, float width);

void flap_rect_set_height(flap_Rect *rect, float height);

void flap_rect_set_size(flap_Rect *rect, float width, float height);


void flap_rect_draw();

int flap_rect_intersect(flap_Rect *r1, flap_Rect *r2);

#endif // FLAP_RECT_H_
