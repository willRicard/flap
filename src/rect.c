#include "flap.h"
#include "rect.h"
#include <inttypes.h>

const float FLAP_BIRD_COLOR[] = {1.f, 1.f, 0.f};
const float FLAP_PIPE_COLOR[] = {0.035f, 0.42f, 0.035f};

static unsigned int count = 0;

static Rect vertices[1 + FLAP_NUM_PIPES * 2];

static const uint16_t indices[] = {
    0,  1,  2,  2,  1,  3,  4,  5,  6,  6,  5,  7,  8,  9,  10, 10, 9,  11,
    12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23,
    24, 25, 26, 26, 25, 27, 28, 29, 27, 27, 29, 31, 32, 33, 34, 34, 33, 34,
};

Rect *rect_new() { return &vertices[count++]; }

Rect *rect_get_vertices() { return vertices; }

const uint16_t *rect_get_indices() { return indices; }

float rect_get_x(Rect *rect) { return rect->vertices[0].x; }

float rect_get_y(Rect *rect) { return rect->vertices[0].y; }

float rect_get_width(Rect *rect) {
  return rect->vertices[1].x - rect->vertices[0].x;
}

float rect_get_height(Rect *rect) {
  return rect->vertices[2].y - rect->vertices[0].y;
}

int rect_intersect(Rect *r1, Rect *r2) {
  float r1_x = rect_get_x(r1);
  float r1_y = rect_get_y(r1);
  float r1_w = rect_get_width(r1);
  float r1_h = rect_get_height(r1);

  float r2_x = rect_get_x(r2);
  float r2_y = rect_get_y(r2);
  float r2_w = rect_get_width(r2);
  float r2_h = rect_get_height(r2);

  return (r1_x < r2_x + r2_w && r2_x < r1_x + r1_w && r1_y < r2_y + r2_h &&
          r2_y < r1_y + r1_h);
}

void rect_set_x(Rect *rect, float x) {
  float dx = x - rect->vertices[0].x;
  rect->vertices[0].x += dx;
  rect->vertices[1].x += dx;
  rect->vertices[2].x += dx;
  rect->vertices[3].x += dx;
}

void rect_set_y(Rect *rect, float y) {
  float dy = y - rect->vertices[0].y;
  rect->vertices[0].y += dy;
  rect->vertices[1].y += dy;
  rect->vertices[2].y += dy;
  rect->vertices[3].y += dy;
}

void rect_set_position(Rect *rect, float x, float y) {
  rect_set_x(rect, x);
  rect_set_y(rect, y);
}

void rect_move(Rect *rect, float x, float y) {
  rect_set_x(rect, rect_get_x(rect) + x);
  rect_set_y(rect, rect_get_y(rect) + y);
}

void rect_set_width(Rect *rect, float width) {
  rect->vertices[1].x = rect_get_x(rect) + width;
  rect->vertices[3].x = rect_get_x(rect) + width;
}

void rect_set_height(Rect *rect, float height) {
  rect->vertices[2].y = rect_get_y(rect) + height;
  rect->vertices[3].y = rect_get_y(rect) + height;
}

void rect_set_size(Rect *rect, float width, float height) {
  rect_set_width(rect, width);
  rect_set_height(rect, height);
}
