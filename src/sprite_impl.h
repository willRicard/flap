#pragma once
#include "sprite.h"

static const int kIndicesPerSprite = 6; // Two triangles

static const float kTextureWidth = 128.F;
static const float kTextureHeight = 32.F;

static unsigned int count = 0;

static Sprite vertices[kNumSprites];

const unsigned short indices[] = {
    0,   1,   2,   2,   0,   3,   4,   5,   6,   6,   4,   7,   8,   9,   10,
    10,  8,   11,  12,  13,  14,  14,  12,  15,  16,  17,  18,  18,  16,  19,
    20,  21,  22,  22,  20,  23,  24,  25,  26,  26,  24,  27,  28,  29,  30,
    30,  28,  31,  32,  33,  34,  34,  32,  35,  36,  37,  38,  38,  36,  39,
    40,  41,  42,  42,  40,  43,  44,  45,  46,  46,  44,  47,  48,  49,  50,
    50,  48,  51,  52,  53,  54,  54,  52,  55,  56,  57,  58,  58,  56,  59,
    60,  61,  62,  62,  60,  63,  64,  65,  66,  66,  64,  67,  68,  69,  70,
    70,  68,  71,  72,  73,  74,  74,  72,  75,  76,  77,  78,  78,  76,  79,
    80,  81,  82,  82,  80,  83,  84,  85,  86,  86,  84,  87,  88,  89,  90,
    90,  88,  91,  92,  93,  94,  94,  92,  95,  96,  97,  98,  98,  96,  99,
    100, 101, 102, 102, 100, 103, 104, 105, 106, 106, 104, 107, 108, 109, 110,
    110, 108, 111, 112, 113, 114, 114, 112, 115, 116, 117, 118, 118, 116, 119,
    120, 121, 122, 122, 120, 123, 124, 125, 126, 126, 124, 127, 128, 129, 130,
    130, 128, 131, 132, 133, 134, 134, 132, 135, 136, 137, 138, 138, 136, 139,
    140, 141, 142, 142, 140, 143, 144, 145, 146, 146, 144, 147, 148, 149, 150,
    150, 148, 151, 152, 153, 154, 154, 152, 155, 156, 157, 158, 158, 156, 159,
    160, 161, 162, 162, 160, 163, 164, 165, 166, 166, 164, 167,
};

Sprite *sprite_new(float texture_x, float texture_y, float texture_w,
                   float texture_h) {
  Sprite *sprite = &vertices[count++];

  const float left_texcoord = texture_x / kTextureWidth;
  const float top_texcoord = texture_y / kTextureHeight;
  const float right_texcoord = (texture_x + texture_w) / kTextureWidth;
  const float bottom_texcoord = (texture_y + texture_h) / kTextureHeight;

  sprite->vertices[0].tx = left_texcoord;
  sprite->vertices[0].ty = top_texcoord;

  sprite->vertices[1].tx = left_texcoord;
  sprite->vertices[1].ty = bottom_texcoord;

  sprite->vertices[2].tx = right_texcoord;
  sprite->vertices[2].ty = bottom_texcoord;

  sprite->vertices[3].tx = right_texcoord;
  sprite->vertices[3].ty = top_texcoord;

  return sprite;
}
