#ifndef FLAP_RECT_H_
#define FLAP_RECT_H_
#include <inttypes.h>

/*
 * A rectangle to be drawn.
 */
typedef struct Rect {
  float x;
  float y;
  float w;
  float h;
} Rect;

void rect_init();

void rect_quit();

void rect_draw();

Rect *rect_new();

Rect *rect_get_vertices();

const uint16_t *rect_get_indices();

int rect_intersect(Rect *r1, Rect *r2);

#endif // FLAP_RECT_H_
