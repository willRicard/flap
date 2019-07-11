#ifndef FLAP_ASSETS_H
#define FLAP_ASSETS_H

#include <stddef.h>

#include <vulkan/vulkan.h>

#include <sulfur/device.h>
#include <sulfur/shader.h>
#include <sulfur/texture.h>

char *assets_base_read_file(const char *file_path, size_t *data_size);

void assets_base_write_file(const char *data, size_t data_size,
                            const char *file_path);

char *assets_read_file(const char *file_path, size_t *data_size);

void assets_write_file(const char *data, size_t data_size,
                       const char *file_path);

VkResult assets_create_shader(SulfurDevice *dev, const char *file_path,
                              VkShaderStageFlags shader_stage,
                              SulfurShader *shader);

VkResult assets_create_texture(SulfurDevice *device, const char *file_path,
                               VkFormat format, SulfurTexture *texture);

VkResult assets_create_pipeline_cache(SulfurDevice *dev, const char *file_path,
                                      VkPipelineCache *pipeline_cache);

void assets_destroy_pipeline_cache(SulfurDevice *dev,
                                   VkPipelineCache pipeline_cache,
                                   const char *file_path);

#endif // FLAP_ASSETS_H
