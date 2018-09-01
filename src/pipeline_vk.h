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

#endif // FLAP_VK_PIPELINE_H_
