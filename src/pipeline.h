#ifndef FLAP_PIPELINE_H_
#define FLAP_PIPELINE_H_
#include <vulkan/vulkan.h>

/**
 * A versatile Pipeline object with convenient defaults.
 */
typedef struct {
  VkPipelineLayout pipeline_layout;
  VkPipeline pipeline;
} Pipeline;

typedef struct PipelineCreateInfo {
  uint32_t num_shader_stages;
  VkPipelineShaderStageCreateInfo *shader_stages;

  uint32_t num_bindings;
  uint32_t num_attributes;
  VkVertexInputBindingDescription *bindings;
  VkVertexInputAttributeDescription *attributes;

  uint32_t num_vertex_attributes;
  uint32_t num_instance_attributes;
  VkVertexInputAttributeDescription vertex_attributes[3];
  VkVertexInputAttributeDescription instance_attributes[2];

  uint32_t num_set_layouts;
  VkDescriptorSetLayout *set_layouts;

  uint32_t num_push_constant_ranges;
  VkPushConstantRange *push_constant_ranges;
} PipelineCreateInfo;

/**
 * Load the pipeline cache from `pipeline_cache.bin`.
 */
void pipeline_cache_init();

/**
 * Write the pipeline cache to `pipeline_cache.bin` then destroy it.
 */
void pipeline_cache_quit();

/**
 * Create the pipeline and pipeline layout.
 */
Pipeline pipeline_create(PipelineCreateInfo *pipeline_info);

/**
 * Destroy the pipeline and pipeline layout.
 */
void pipeline_destroy(Pipeline pipeline);

#endif // FLAP_PIPELINE_H_
