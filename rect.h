#ifndef FLAP_RECT_H_
#define FLAP_RECT_H_

#include <graphene.h>

typedef struct flap_Rect {
  graphene_vec2_t pos;
  graphene_vec2_t size;
  graphene_vec3_t color;
} flap_Rect;

void flap_rect_init();

void flap_rect_quit();

flap_Rect *flap_rect_new();

void flap_rect_draw();

int flap_rect_intersect(flap_Rect *r1, flap_Rect *r2);

#endif // FLAP_RECT_H_
