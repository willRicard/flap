#ifndef FLAP_VK_PIPELINE_H_
#define FLAP_VK_PIPELINE_H_

#include <vulkan/vulkan.h>

typedef struct {
  VkShaderModule vertexModule;
  VkShaderModule fragmentModule;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
} flapPipeline;

flapPipeline flapPipelineCreate(const char *vertexShaderSource,
                                const char *fragmentShaderSource);

void flapPipelineDestroy(flapPipeline pipeline);

#endif // FLAP_VK_PIPELINE_H_
