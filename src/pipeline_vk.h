#ifndef FLAP_VK_PIPELINE_H_
#define FLAP_VK_PIPELINE_H_
#include "pipeline.h"

#include <vulkan/vulkan.h>

typedef struct Pipeline {
  VkShaderModule vertex_module;
  VkShaderModule fragment_module;
  VkPipelineLayout pipeline_layout;
  VkPipeline pipeline;
} Pipeline;

void vulkan_destroy_pipeline_cache();

// Create the pipeline and pipeline layout (keep shader modules).
void vulkan_pipeline_create(Pipeline *pipeline);

// Destroy the pipeline and pipeline layout (keep shader modules).
void vulkan_pipeline_destroy(Pipeline *pipeline);

#endif // FLAP_VK_PIPELINE_H_
