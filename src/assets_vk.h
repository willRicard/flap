#pragma once
#include "assets.h"

#include <vulkan/vulkan.h>

#include <sulfur/device.h>
#include <sulfur/shader.h>
#include <sulfur/texture.h>

VkResult assets_vk_create_shader(SulfurDevice *dev, const char *file_path,
                                 VkShaderStageFlags shader_stage,
                                 SulfurShader *shader);

VkResult assets_vk_create_texture(SulfurDevice *device, const char *file_path,
                                  VkFormat format, SulfurTexture *texture);

VkResult assets_vk_create_pipeline_cache(SulfurDevice *dev,
                                         const char *file_path,
                                         VkPipelineCache *pipeline_cache);

void assets_vk_destroy_pipeline_cache(SulfurDevice *dev,
                                      VkPipelineCache pipeline_cache,
                                      const char *file_path);
