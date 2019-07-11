#ifndef FLAP_SPRITE_H
#define FLAP_SPRITE_H
#include <vulkan/vulkan.h>

#include <sulfur/device.h>

#define FLAP_SPRITE_TEXTURE_BIRD_X 0
#define FLAP_SPRITE_TEXTURE_BIRD_Y 0
#define FLAP_SPRITE_TEXTURE_BIRD_WIDTH 32
#define FLAP_SPRITE_TEXTURE_BIRD_HEIGHT 32

#define FLAP_SPRITE_TEXTURE_PIPE_HEAD_X 64
#define FLAP_SPRITE_TEXTURE_PIPE_HEAD_Y 0
#define FLAP_SPRITE_TEXTURE_PIPE_HEAD_WIDTH 32
#define FLAP_SPRITE_TEXTURE_PIPE_HEAD_HEIGHT 16

#define FLAP_SPRITE_TEXTURE_PIPE_BODY_X 102
#define FLAP_SPRITE_TEXTURE_PIPE_BODY_Y 0
#define FLAP_SPRITE_TEXTURE_PIPE_BODY_WIDTH 20
#define FLAP_SPRITE_TEXTURE_PIPE_BODY_HEIGHT 32

#define FLAP_SPRITE_PIPE_HEAD_HEIGHT 16.F / 32.F

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
Sprite *sprite_new(int x, int y, int w, int h);

void sprite_set_x(Sprite *sprite, float x);

void sprite_set_y(Sprite *sprite, float y);

void sprite_set_w(Sprite *sprite, float w);

void sprite_set_h(Sprite *sprite, float h);

void sprite_set_th(Sprite *sprite, float th);

float sprite_get_x(Sprite *sprite);

float sprite_get_y(Sprite *sprite);

float sprite_get_w(Sprite *sprite);

float sprite_get_h(Sprite *sprite);

/**
 * Collision detection.
 */
int sprite_intersect(Sprite *s1, Sprite *s2);

#endif // FLAP_SPRITE_H
