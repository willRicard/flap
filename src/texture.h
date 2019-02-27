#ifndef FLAP_IMAGE_H_
#define FLAP_IMAGE_H_
#include "device.h"
#include <vulkan/vulkan.h>

typedef struct {
  VkImage image;
  VkDeviceMemory image_memory;
  VkImageView image_view;
  VkSampler sampler;
  int width;
  int height;
} Texture;

/**
 * Create an empty texture.
 */
void texture_create(Device *device, int width, int height, VkFormat format,
                    Texture *texture);

/**
 * Create a texture from an image file.
 */
void texture_create_from_image(Device *device, const char *filename,
                               VkFormat format, Texture *texture);

/**
 * Destroy a texture.
 */
void texture_destroy(Device *device, Texture *texture);

/**
 * Transition the texture image to a given VkImageLayout.
 */
void texture_transition_layout(Device *device, Texture *texture,
                               VkImageLayout old_layout,
                               VkImageLayout new_layout);

#endif // FLAP_IMAGE_H_
