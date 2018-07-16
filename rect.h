#ifndef FLAP_RECT_H_
#define FLAP_RECT_H_
#include <inttypes.h>

typedef struct {
  float x;
  float y;
  float w;
  float h;
} flapVertex;

typedef struct flapRect {
  flapVertex vertices[4];
} flapRect;

flapRect *flapRectNew();

flapRect *flapRectGetVertices();

const uint16_t *flapRectGetIndices();

float flapRectGetX(flapRect *rect);

float flapRectGetY(flapRect *rect);

float flapRectGetWidth(flapRect *rect);

float flapRectGetHeight(flapRect *rect);

int flapRectIntersect(flapRect *r1, flapRect *r2);

void flapRectSetX(flapRect *rect, float x);

void flapRectSetY(flapRect *rect, float y);

void flapRectSetPosition(flapRect *rect, float x, float y);

void flapRectMove(flapRect *rect, float x, float y);

void flapRectSetWidth(flapRect *rect, float width);

void flapRectSetHeight(flapRect *rect, float height);

void flapRectSetSize(flapRect *rect, float width, float height);

// The following methods must be implemented by the render system.

void flapRectInit();

void flapRectQuit();

void flapRectDraw();

#endif // FLAP_RECT_H_
