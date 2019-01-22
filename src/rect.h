#ifndef FLAP_RECT_H_
#define FLAP_RECT_H_
#include <vulkan/vulkan.h>

#include "device.h"

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
 * Load shaders
 */
void rect_init(Device *device);

/**
 * Free shaders
 */
void rect_quit(Device *device);

/**
 * Build pipeline create info.
 */
void rect_get_pipeline_create_info(VkGraphicsPipelineCreateInfo *pipeline_info);

/**
 * Bake rendering commands.
 */
void rect_record_command_buffer(VkCommandBuffer cmd_buf);

/**
 * Send vertex data to the GPU.
 */
void rect_update();

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
