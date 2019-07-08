#ifndef FLAP_ASSETS_H
#define FLAP_ASSETS_H

#include <stddef.h>

#include <vulkan/vulkan.h>

#include <sulfur/device.h>
#include <sulfur/shader.h>
#include <sulfur/texture.h>

/**
 * Read the contents of a file into memory.
 * You are responsible for freeing the allocated memory.
 */
char *assets_read_file(const char *file_path, size_t *data_size);

void assets_write_file(const char *data, size_t data_size,
                       const char *file_path);

VkResult assets_create_shader(SulfurDevice *dev, const char *file_path,
                              VkShaderStageFlags shader_stage,
                              SulfurShader *shader);

VkResult assets_create_texture(SulfurDevice *device, const char *file_path,
                               VkFormat format, SulfurTexture *texture);

/**
 * Load the pipeline cache from `pipeline_cache.bin`.
 */
VkResult assets_create_pipeline_cache(SulfurDevice *dev,
                                      VkPipelineCache *pipeline_cache);

/**
 * Write the pipeline cache to `pipeline_cache.bin` then destroy it.
 */
void assets_destroy_pipeline_cache(SulfurDevice *dev,
                                   VkPipelineCache pipeline_cache);

#endif // FLAP_ASSETS_H
