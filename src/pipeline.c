#include "pipeline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "assets.h"
#include "error.h"
#include "renderer.h"

static VkPipelineCache pipeline_cache = VK_NULL_HANDLE;

void pipeline_cache_init() {
  FILE *cache_file = NULL;
#ifdef _WIN32
  fopen_s(&cache_file, "pipeline_cache.bin", "rb");
#else
  cache_file = fopen("pipeline_cache.bin", "rb");
#endif

  if (cache_file == NULL) {
    return;
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

  if (vkCreatePipelineCache(renderer_get_device(), &cache_info, NULL,
                            &pipeline_cache) != VK_SUCCESS) {
    fail_with_error("Error creating pipeline cache.");
  }
  free(initial_data);
}

void pipeline_cache_quit() {
  FILE *cache_file = NULL;
#ifdef _WIN32
  fopen_s(&cache_file, "pipeline_cache.bin", "wb");
#else
  cache_file = fopen("pipeline_cache.bin", "wb");
#endif
  if (cache_file != NULL) {

    size_t size = 0;

    vkGetPipelineCacheData(renderer_get_device(), pipeline_cache, &size, NULL);

    char *data = (char *)malloc(size * sizeof(char));

    vkGetPipelineCacheData(renderer_get_device(), pipeline_cache, &size, data);

    fwrite(data, size, 1, cache_file);

    fclose(cache_file);
  }

  vkDestroyPipelineCache(renderer_get_device(), pipeline_cache, NULL);
}

Pipeline pipeline_create(PipelineCreateInfo *info) {
  Pipeline pipeline = {VK_NULL_HANDLE};

  VkDevice device = renderer_get_device();

  VkPipelineVertexInputStateCreateInfo input_info = {0};
  input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  input_info.vertexBindingDescriptionCount = info->num_bindings;
  input_info.pVertexBindingDescriptions = info->bindings;

  input_info.vertexAttributeDescriptionCount = info->num_attributes;
  input_info.pVertexAttributeDescriptions = info->attributes;

  VkPipelineInputAssemblyStateCreateInfo assembly_info = {0};
  assembly_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  assembly_info.pNext = NULL;
  assembly_info.flags = 0;
  assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
  assembly_info.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport = {0};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)renderer_get_extent().width;
  viewport.height = (float)renderer_get_extent().height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {0};
  scissor.extent = renderer_get_extent();
  scissor.offset.x = 0;
  scissor.offset.y = 0;

  VkPipelineViewportStateCreateInfo viewport_info = {0};
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.pNext = NULL;
  viewport_info.flags = 0;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = &viewport;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterization = {0};
  rasterization.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterization.pNext = NULL;
  rasterization.flags = 0;
  rasterization.depthClampEnable = VK_FALSE;
  rasterization.rasterizerDiscardEnable = VK_FALSE;
  rasterization.polygonMode = VK_POLYGON_MODE_FILL;
  rasterization.cullMode = VK_CULL_MODE_NONE;
  rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterization.depthBiasEnable = VK_FALSE;
  rasterization.depthBiasConstantFactor = 0.0f;
  rasterization.depthBiasClamp = 0.0f;
  rasterization.depthBiasSlopeFactor = 0.0f;
  rasterization.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisample = {0};
  multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample.pNext = NULL;
  multisample.flags = 0;
  multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisample.sampleShadingEnable = VK_FALSE;
  multisample.minSampleShading = 0.0f;
  multisample.pSampleMask = NULL;
  multisample.alphaToCoverageEnable = VK_FALSE;
  multisample.alphaToOneEnable = VK_FALSE;

  VkPipelineDepthStencilStateCreateInfo depth_info = {0};
  depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_info.pNext = NULL;
  depth_info.flags = 0;
  depth_info.depthTestEnable = VK_FALSE;
  depth_info.depthWriteEnable = VK_FALSE;
  depth_info.depthCompareOp = 0;
  depth_info.depthBoundsTestEnable = VK_FALSE;
  depth_info.stencilTestEnable = VK_FALSE;
  depth_info.depthTestEnable = VK_FALSE;
  depth_info.depthTestEnable = VK_FALSE;
  depth_info.depthTestEnable = VK_FALSE;
  depth_info.minDepthBounds = 0.f;
  depth_info.maxDepthBounds = 1.f;

  VkPipelineColorBlendAttachmentState blend_attachment = {0};
  blend_attachment.blendEnable = VK_FALSE;
  blend_attachment.srcColorBlendFactor = 0.0f;
  blend_attachment.dstAlphaBlendFactor = 0.0f;
  blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  blend_attachment.srcAlphaBlendFactor = 0.0f;
  blend_attachment.dstAlphaBlendFactor = 0.0f;
  blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
  blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo blend_info = {0};
  blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend_info.pNext = NULL;
  blend_info.flags = 0;
  blend_info.logicOpEnable = VK_FALSE;
  blend_info.logicOp = VK_LOGIC_OP_COPY;
  blend_info.attachmentCount = 1;
  blend_info.pAttachments = &blend_attachment;

  VkPipelineLayoutCreateInfo layout_info = {0};
  layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layout_info.pNext = NULL;
  layout_info.flags = 0;
  layout_info.setLayoutCount = info->num_set_layouts;
  layout_info.pSetLayouts = info->set_layouts;
  layout_info.pushConstantRangeCount = info->num_push_constant_ranges;
  layout_info.pPushConstantRanges = info->push_constant_ranges;

  if (vkCreatePipelineLayout(device, &layout_info, NULL,
                             &pipeline.pipeline_layout) != VK_SUCCESS) {
    fail_with_error("An error occured while creating the pipeline layout.");
  }

  VkGraphicsPipelineCreateInfo pipeline_info = {0};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.pNext = NULL;
  pipeline_info.flags = 0;

  pipeline_info.stageCount = info->num_shader_stages;
  pipeline_info.pStages = info->shader_stages,

  pipeline_info.pVertexInputState = &input_info;
  pipeline_info.pInputAssemblyState = &assembly_info;
  pipeline_info.pTessellationState = NULL;
  pipeline_info.pViewportState = &viewport_info;
  pipeline_info.pRasterizationState = &rasterization;
  pipeline_info.pMultisampleState = &multisample;
  pipeline_info.pDepthStencilState = &depth_info;
  pipeline_info.pColorBlendState = &blend_info;
  pipeline_info.pDynamicState = NULL;
  pipeline_info.layout = pipeline.pipeline_layout;
  pipeline_info.renderPass = renderer_get_render_pass();
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(device, pipeline_cache, 1, &pipeline_info, NULL,
                                &pipeline.pipeline) != VK_SUCCESS) {
    fail_with_error("An error occured while creating the graphics pipeline.");
  }

  return pipeline;
}

void pipeline_destroy(Pipeline pipeline) {
  VkDevice device = renderer_get_device();

  vkDestroyPipelineLayout(device, pipeline.pipeline_layout, NULL);
  vkDestroyPipeline(device, pipeline.pipeline, NULL);
}
