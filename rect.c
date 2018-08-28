#include "flap.h"
#include "rect.h"
#include <inttypes.h>

const float FLAP_BIRD_COLOR[] = {1.f, 1.f, 0.f};
const float FLAP_PIPE_COLOR[] = {0.035f, 0.42f, 0.035f};

static unsigned int count = 0;

static flapRect vertices[1 + FLAP_NUM_PIPES * 2];

static const uint16_t indices[] = {
    0,  1,  2,  2,  1,  3,  4,  5,  6,  6,  5,  7,  8,  9,  10, 10, 9,  11,
    12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23,
    24, 25, 26, 26, 25, 27, 28, 29, 27, 27, 29, 31, 32, 33, 34, 34, 33, 34,
};

flapRect *flapRectNew() {
  return &vertices[count++];
}

flapRect *flapRectGetVertices() { return vertices; }

const uint16_t *flapRectGetIndices() { return indices; }

float flapRectGetX(flapRect *rect) { return rect->vertices[0].x; }

float flapRectGetY(flapRect *rect) { return rect->vertices[0].y; }

float flapRectGetWidth(flapRect *rect) {
  return rect->vertices[1].x - rect->vertices[0].x;
}

float flapRectGetHeight(flapRect *rect) {
  return rect->vertices[2].y - rect->vertices[0].y;
}

int flapRectIntersect(flapRect *r1, flapRect *r2) {
  float r1_x = flapRectGetX(r1);
  float r1_y = flapRectGetY(r1);
  float r1_w = flapRectGetWidth(r1);
  float r1_h = flapRectGetHeight(r1);

  float r2_x = flapRectGetX(r2);
  float r2_y = flapRectGetY(r2);
  float r2_w = flapRectGetWidth(r2);
  float r2_h = flapRectGetHeight(r2);

  return (r1_x < r2_x + r2_w && r2_x < r1_x + r1_w && r1_y < r2_y + r2_h &&
          r2_y < r1_y + r1_h);
}

void flapRectSetX(flapRect *rect, float x) {
  float dx = x - rect->vertices[0].x;
  rect->vertices[0].x += dx;
  rect->vertices[1].x += dx;
  rect->vertices[2].x += dx;
  rect->vertices[3].x += dx;
}

void flapRectSetY(flapRect *rect, float y) {
  float dy = y - rect->vertices[0].y;
  rect->vertices[0].y += dy;
  rect->vertices[1].y += dy;
  rect->vertices[2].y += dy;
  rect->vertices[3].y += dy;
}

void flapRectSetPosition(flapRect *rect, float x, float y) {
  flapRectSetX(rect, x);
  flapRectSetY(rect, y);
}

void flapRectMove(flapRect *rect, float x, float y) {
  flapRectSetX(rect, flapRectGetX(rect) + x);
  flapRectSetY(rect, flapRectGetY(rect) + y);
}

void flapRectSetWidth(flapRect *rect, float width) {
  rect->vertices[1].x = flapRectGetX(rect) + width;
  rect->vertices[3].x = flapRectGetX(rect) + width;
}

void flapRectSetHeight(flapRect *rect, float height) {
  rect->vertices[2].y = flapRectGetY(rect) + height;
  rect->vertices[3].y = flapRectGetY(rect) + height;
}

void flapRectSetSize(flapRect *rect, float width, float height) {
  flapRectSetWidth(rect, width);
  flapRectSetHeight(rect, height);
}
