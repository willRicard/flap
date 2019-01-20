#ifndef FLAP_RECT_H_
#define FLAP_RECT_H_
#include <inttypes.h>

/**
 * A coloured rectangle
 */
typedef struct Rect {
  float x;
  float y;
  float w;
  float h;
} Rect;

/**
 * Create the Rect GPU pipeline and record command buffers.
 */
void rect_init();

/**
 * Free
 */
void rect_quit();

/**
 * Send vertex data to the GPU.
 */
void rect_draw();

/**
 * Make a new rect.
 */
Rect *rect_new();

/**
 * Retrieve all Rects.
 */
Rect *rect_get_vertices();

/**
 * Collision test.
 */
int rect_intersect(Rect *r1, Rect *r2);

#endif // FLAP_RECT_H_
