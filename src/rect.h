#ifndef FLAP_RECT_H_
#define FLAP_RECT_H_
#include <inttypes.h>

/*
 * A rectangle to be drawn.
 */
typedef struct {
  float x;
  float y;
  float w;
  float h;
} Vertex;

typedef struct Rect {
  Vertex vertices[4];
} Rect;

Rect *rect_new();

Rect *rect_get_vertices();

const uint16_t *rect_get_indices();

float rect_get_x(Rect *rect);

float rect_get_y(Rect *rect);

float rect_get_width(Rect *rect);

float rect_get_height(Rect *rect);

int rect_intersect(Rect *r1, Rect *r2);

void rect_set_x(Rect *rect, float x);

void rect_set_y(Rect *rect, float y);

void rect_set_position(Rect *rect, float x, float y);

void rect_move(Rect *rect, float x, float y);

void rect_set_width(Rect *rect, float width);

void rect_set_height(Rect *rect, float height);

void rect_set_size(Rect *rect, float width, float height);

// The following methods must be implemented by the render system.

void rect_init();

void rect_quit();

void rect_draw();

#endif // FLAP_RECT_H_
