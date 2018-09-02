#ifndef FLAP_PIPELINE_H_
#define FLAP_PIPELINE_H_
#include <vulkan/vulkan.h>

typedef struct Pipeline {
  uint32_t num_shader_stages;
  VkPipelineShaderStageCreateInfo shader_stages[3];

  uint32_t num_attributes;
  VkVertexInputAttributeDescription attribute_descriptions[2];

  VkPipelineLayout pipeline_layout;
  VkPipeline pipeline;
} Pipeline;

// Load the pipeline cache from `pipeline_cache.bin`
void pipeline_cache_init();

// Write the pipeline cache to `pipeline_cache.bin`
// then destroy it
void pipeline_cache_quit();

// Compile a shader from SPIR-V source.
VkShaderModule shader_create(const char *file_name);

void shader_destroy(VkShaderModule module);

void pipeline_add_shader(Pipeline *pipeline, VkShaderModule shader_module,
                         VkShaderStageFlags shader_stage);

void pipeline_add_attribute(Pipeline *pipeline, VkFormat format, uint32_t stride);

// Create the pipeline and pipeline layout.
void pipeline_create(Pipeline *pipeline);

// Destroy the pipeline and pipeline layout (keep shader modules).
void pipeline_destroy(Pipeline pipeline);

#endif // FLAP_PIPELINE_H_
