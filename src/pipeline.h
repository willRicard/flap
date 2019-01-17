#ifndef FLAP_PIPELINE_H_
#define FLAP_PIPELINE_H_
#include <vulkan/vulkan.h>

// A versatile Pipeline object with convenient defaults
typedef struct Pipeline {
  uint32_t num_shader_stages;
  VkPipelineShaderStageCreateInfo shader_stages[3];

  uint32_t num_bindings;
  uint32_t num_attributes;
  VkVertexInputBindingDescription *bindings;
  VkVertexInputAttributeDescription *attributes;

  uint32_t num_vertex_attributes;
  uint32_t num_instance_attributes;
  uint32_t vertex_attribute_stride;
  uint32_t instance_attribute_stride;
  VkVertexInputAttributeDescription vertex_attributes[3];
  VkVertexInputAttributeDescription instance_attributes[2];

  uint32_t num_set_layouts;
  VkDescriptorSetLayout set_layouts[1];

  uint32_t num_push_constants;
  VkPushConstantRange push_constants[1];

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

// Destroy a compiled shader.
void shader_destroy(VkShaderModule module);

// Add a shader stage to the pipeline.
void pipeline_add_shader(Pipeline *pipeline, VkShaderModule shader_module,
                         VkShaderStageFlags shader_stage);

// Add vertex or instance attributes
void pipeline_add_attributes(Pipeline *pipeline,
                             uint32_t num_attribute_bindings,
                             VkVertexInputBindingDescription *bindings,
                             uint32_t num_attributes,
                             VkVertexInputAttributeDescription *attributes);

// Add a uniform buffer object to a given shader stage.
void pipeline_add_uniform_buffer(Pipeline *pipeline,
                                 VkShaderStageFlags shader_stage);

// Add a push constant range to a given shader stage.
void pipeline_add_push_constant(Pipeline *pipeline,
                                VkShaderStageFlags shader_stage,
                                VkDeviceSize offset, VkDeviceSize size);

// Add a descriptor set layout.
void pipeline_add_set_layout(Pipeline *pipeline,
                             VkDescriptorSetLayout set_layout);

// Create the pipeline and pipeline layout.
void pipeline_create(Pipeline *pipeline);

// Destroy the pipeline and pipeline layout (keep shader modules).
void pipeline_destroy(Pipeline pipeline);

#endif // FLAP_PIPELINE_H_
