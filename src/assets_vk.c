#include "assets_vk.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#include "stb_image.h"

/**
 * Read SPIR-V code from `file_path`.
 */
VkResult assets_vk_create_shader(SulfurDevice *dev, const char *file_path,
                                 VkShaderStageFlags shader_stage,
                                 SulfurShader *shader) {
  size_t shader_code_size = 0;
  char *shader_code = assets_read_file(file_path, &shader_code_size);
  if (shader_code == NULL) {
    return -1;
  }

  VkResult result = sulfur_shader_create(dev, shader_code, shader_code_size,
                                         shader_stage, shader);

  free(shader_code);

  return result;
}

/**
 * Read pipeline cache data from `file_path`.
 */
VkResult assets_vk_create_pipeline_cache(SulfurDevice *dev,
                                         const char *file_path,
                                         VkPipelineCache *pipeline_cache) {
  char *initial_data = NULL;
  size_t initial_data_size = 0;
  initial_data = assets_read_file(file_path, &initial_data_size);

  // It is OK for `initial_data` to be NULL
  // since we would create an empty cache.

  VkPipelineCacheCreateInfo cache_info = {0};
  cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  cache_info.pNext = NULL;
  cache_info.flags = 0;
  cache_info.initialDataSize = initial_data_size;
  cache_info.pInitialData = initial_data;

  VkResult result =
      vkCreatePipelineCache(dev->device, &cache_info, NULL, pipeline_cache);

  free(initial_data);

  return result;
}

/**
 * Write pipeline cache data to `file_path`.
 */
void assets_vk_destroy_pipeline_cache(SulfurDevice *dev,
                                      VkPipelineCache pipeline_cache,
                                      const char *file_path) {
  if (pipeline_cache == VK_NULL_HANDLE) {
    return;
  }

  size_t data_size = 0;
  vkGetPipelineCacheData(dev->device, pipeline_cache, &data_size, NULL);

  char *data = (char *)malloc(data_size * sizeof(char));
  vkGetPipelineCacheData(dev->device, pipeline_cache, &data_size, data);

  assets_write_file(data, data_size, file_path);

  vkDestroyPipelineCache(dev->device, pipeline_cache, NULL);
}

/**
 * Read image data from `file_path`.
 */
VkResult assets_vk_create_texture(SulfurDevice *dev, const char *file_path,
                                  VkFormat format, SulfurTexture *texture) {
  size_t data_size;
  char *data = NULL;
  data = assets_read_file(file_path, &data_size);
  if (data == NULL) {
    return -1;
  }

  int width = 0, height = 0;
  stbi_uc *pixels = stbi_load_from_memory(
      (const stbi_uc *)data, data_size, &width, &height, NULL, STBI_rgb_alpha);

  free(data);

  if (pixels == NULL) {
    return -1;
  }

  VkResult result = sulfur_texture_create_from_image(dev, format, width, height,
                                                     pixels, texture);
  stbi_image_free(pixels);
  return result;
}
