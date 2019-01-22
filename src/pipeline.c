#include "pipeline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "assets.h"
#include "error.h"

VkPipelineCache pipeline_cache_create(Device *device) {
  FILE *cache_file = NULL;
#ifdef _WIN32
  fopen_s(&cache_file, "pipeline_cache.bin", "rb");
#else
  cache_file = fopen("pipeline_cache.bin", "rb");
#endif

  if (cache_file == NULL) {
    return VK_NULL_HANDLE;
  }

  fseek(cache_file, SEEK_END, 0);
  long initial_data_size = ftell(cache_file);

  rewind(cache_file);

  char *initial_data = (char *)malloc(initial_data_size * sizeof(char));
  fread(initial_data, initial_data_size, 1, cache_file);

  VkPipelineCacheCreateInfo cache_info = {0};
  cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  cache_info.pNext = NULL;
  cache_info.flags = 0;
  cache_info.initialDataSize = initial_data_size;
  cache_info.pInitialData = initial_data;

  VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
  error_check(
      vkCreatePipelineCache(device->device, &cache_info, NULL, &pipeline_cache),
      "vkCreatePipelineCache");
  free(initial_data);

  return pipeline_cache;
}

void pipeline_cache_destroy(Device *device, VkPipelineCache pipeline_cache) {
  if (pipeline_cache == VK_NULL_HANDLE) {
    return;
  }
  FILE *cache_file = NULL;
#ifdef _WIN32
  fopen_s(&cache_file, "pipeline_cache.bin", "wb");
#else
  cache_file = fopen("pipeline_cache.bin", "wb");
#endif
  if (cache_file == NULL) {
    vkDestroyPipelineCache(device->device, pipeline_cache, NULL);
    return;
  }

  size_t size = 0;

  vkGetPipelineCacheData(device->device, pipeline_cache, &size, NULL);

  char *data = (char *)malloc(size * sizeof(char));

  vkGetPipelineCacheData(device->device, pipeline_cache, &size, data);

  fwrite(data, size, 1, cache_file);

  free(data);

  fclose(cache_file);

  vkDestroyPipelineCache(device->device, pipeline_cache, NULL);
}

void pipeline_make_default_create_info(Swapchain *swapchain,
                                       VkGraphicsPipelineCreateInfo *info) {
  static VkPipelineVertexInputStateCreateInfo input_info = {0};
  input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  static VkPipelineInputAssemblyStateCreateInfo assembly_info = {0};
  assembly_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  assembly_info.primitiveRestartEnable = VK_FALSE;

  VkExtent2D swapchain_extent = swapchain->info.imageExtent;

  static VkViewport viewport = {0};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapchain_extent.width;
  viewport.height = (float)swapchain_extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  static VkRect2D scissor = {0};
  scissor.extent = swapchain_extent;
  scissor.offset.x = 0;
  scissor.offset.y = 0;

  static VkPipelineViewportStateCreateInfo viewport_info = {0};
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = &viewport;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = &scissor;

  static VkPipelineRasterizationStateCreateInfo rasterization = {0};
  rasterization.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterization.polygonMode = VK_POLYGON_MODE_FILL;
  rasterization.cullMode = VK_CULL_MODE_NONE;
  rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterization.lineWidth = 1.0f;

  static VkPipelineMultisampleStateCreateInfo multisample = {0};
  multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  static VkPipelineDepthStencilStateCreateInfo depth_info = {0};
  depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_info.maxDepthBounds = 1.f;

  static VkPipelineColorBlendAttachmentState blend_attachment = {0};
  blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
  blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  static VkPipelineColorBlendStateCreateInfo blend_info = {0};
  blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend_info.logicOp = VK_LOGIC_OP_COPY;
  blend_info.attachmentCount = 1;
  blend_info.pAttachments = &blend_attachment;

  info->sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  info->pVertexInputState = &input_info;
  info->pInputAssemblyState = &assembly_info;
  info->pTessellationState = NULL;
  info->pViewportState = &viewport_info;
  info->pRasterizationState = &rasterization;
  info->pMultisampleState = &multisample;
  info->pDepthStencilState = &depth_info;
  info->pColorBlendState = &blend_info;
  info->pDynamicState = NULL;
  info->renderPass = swapchain->render_pass;
  info->subpass = 0;
  info->basePipelineHandle = VK_NULL_HANDLE;
  info->basePipelineIndex = -1;
}
