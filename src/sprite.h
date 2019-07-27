#ifndef FLAP_SPRITE_H
#define FLAP_SPRITE_H
#include <vulkan/vulkan.h>

#include <sulfur/device.h>

/**
 * A sprite vertex.
 */
typedef struct SpriteVertex {
  float x;  // Position
  float y;  // Position
  float tx; // Texture coordinates
  float ty; // Texture coordinates
} SpriteVertex;

/**
 * A textured sprite.
 */
typedef struct Sprite {
  SpriteVertex vertices[4];
} Sprite;

/**
 * Load shaders and resources.
 */
void sprite_init(SulfurDevice *dev);

/**
 * Free shaders and resources.
 */
void sprite_quit(SulfurDevice *device);

/**
 * Build pipeline create info.
 */
void sprite_get_pipeline_create_info(
    VkGraphicsPipelineCreateInfo *pipeline_info);

VkPipelineLayout sprite_get_pipeline_layout(void);

/**
 * Get the descriptor set layout for the texture.
 */
VkDescriptorSetLayout sprite_get_descriptor_set_layout(void);

/**
 * Update vertex data.
 */
void sprite_update(void);

/**
 * Bake rendering commands.
 */
void sprite_record_command_buffer(VkCommandBuffer cmd_buf);

/**
 * Create a descriptor set for the texture.
 */
void sprite_create_descriptor(SulfurDevice *dev,
                              VkDescriptorSet descriptor_set);

/**
 * Make a new sprite from a portion of the texture.
 */
Sprite *sprite_new(float texture_x, float texture_y, float texture_w,
                   float texture_h);

static inline void sprite_set_x(Sprite *sprite, float left) {
  const float width = sprite->vertices[2].x - sprite->vertices[0].x;
  const float right = left + width;
  sprite->vertices[0].x = left;
  sprite->vertices[1].x = left;
  sprite->vertices[2].x = right;
  sprite->vertices[3].x = right;
}

static inline void sprite_set_y(Sprite *sprite, float top) {
  const float height = sprite->vertices[1].y - sprite->vertices[0].y;
  const float bottom = top + height;
  sprite->vertices[0].y = top;
  sprite->vertices[1].y = bottom;
  sprite->vertices[2].y = bottom;
  sprite->vertices[3].y = top;
}

static inline void sprite_set_w(Sprite *sprite, float w) {
  float x = sprite->vertices[0].x;
  sprite->vertices[2].x = x + w;
  sprite->vertices[3].x = x + w;
}

static inline void sprite_set_h(Sprite *sprite, float h) {
  float y = sprite->vertices[0].y;
  sprite->vertices[1].y = y + h;
  sprite->vertices[2].y = y + h;
}

static inline void sprite_set_th(Sprite *sprite, float th) {
  float ty = sprite->vertices[0].ty;
  sprite->vertices[1].ty = ty + th;
  sprite->vertices[2].ty = ty + th;
}

static inline const float sprite_get_x(Sprite *sprite) {
  return sprite->vertices[0].x;
}

static inline const float sprite_get_right(Sprite *sprite) {
  return sprite->vertices[2].x;
}

static inline const float sprite_get_y(Sprite *sprite) {
  return sprite->vertices[0].y;
}

static inline const float sprite_get_bottom(Sprite *sprite) {
  return sprite->vertices[1].y;
}

static inline const float sprite_get_w(Sprite *sprite) {
  return sprite->vertices[2].x - sprite->vertices[1].x;
}

static inline const float sprite_get_h(Sprite *sprite) {
  return sprite->vertices[1].y - sprite->vertices[0].y;
}

/**
 * Collision detection.
 */
static inline const int sprite_intersect(Sprite *s1, Sprite *s2) {
  const float left1 = sprite_get_x(s1);
  const float top1 = sprite_get_y(s1);
  const float right1 = sprite_get_right(s1);
  const float bottom1 = sprite_get_bottom(s1);
  const float left2 = sprite_get_x(s2);
  const float top2 = sprite_get_y(s2);
  const float right2 = sprite_get_right(s2);
  const float bottom2 = sprite_get_bottom(s2);
  return (left1 < right2 && left2 < right1 && top1 < bottom2 && top2 < bottom1);
}

#endif // FLAP_SPRITE_H
