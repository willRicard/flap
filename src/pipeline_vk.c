#include "pipeline_vk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "assets.h"
#include "renderer_vk.h"
#include "window.h"

static VkPipelineCache pipeline_cache = VK_NULL_HANDLE;

static void load_pipeline_cache() {
  uint32_t *initial_data = NULL;
  size_t initial_data_size = 0;
  initial_data =
      (uint32_t *)assets_read_file("pipeline_cache.bin", &initial_data_size);

  VkPipelineCacheCreateInfo pipeline_cache_create_info = {0};
  pipeline_cache_create_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  pipeline_cache_create_info.pNext = NULL;
  pipeline_cache_create_info.flags = 0;
  pipeline_cache_create_info.initialDataSize = initial_data_size;
  pipeline_cache_create_info.pInitialData = initial_data;

  if (vkCreatePipelineCache(renderer_get_device(), &pipeline_cache_create_info,
                            NULL, &pipeline_cache) != VK_SUCCESS) {
    window_fail_with_error("Error creating pipeline cache.");
  }
  free(initial_data);
}

void vulkan_destroy_pipeline_cache() {
  vkDestroyPipelineCache(renderer_get_device(), pipeline_cache, NULL);
}

static void write_pipeline_cache() {
  FILE *cacheFile = NULL;
#ifdef _WIN32
  fopen_s(&cacheFile, "pipeline_cache.bin", "wb");
#else
  cacheFile = fopen("pipeline_cache.bin", "wb");
#endif
  if (cacheFile == NULL) {
    return;
  }

  size_t size;

  vkGetPipelineCacheData(renderer_get_device(), pipeline_cache, &size, NULL);

  char *data = (char *)malloc(size * sizeof(char));

  vkGetPipelineCacheData(renderer_get_device(), pipeline_cache, &size, data);

  fwrite(data, size, 1, cacheFile);

  fclose(cacheFile);
}

static VkShaderModule compile_shader(const char *shaderFile) {

  VkShaderModuleCreateInfo shaderModuleCreateInfo = {0};
  shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderModuleCreateInfo.pNext = NULL;
  shaderModuleCreateInfo.flags = 0;

  uint32_t *shaderCode = NULL;
  shaderCode = (uint32_t *)assets_read_file(shaderFile,
                                            &shaderModuleCreateInfo.codeSize);
  if (shaderCode == NULL) {
    window_fail_with_error("Failed reading shader code.");
  }
  shaderModuleCreateInfo.pCode = (const uint32_t *)shaderCode;

  VkShaderModule shaderModule = VK_NULL_HANDLE;
  if (vkCreateShaderModule(renderer_get_device(), &shaderModuleCreateInfo, NULL,
                           &shaderModule) != VK_SUCCESS) {
    window_fail_with_error("Error while creating the vertex shader module.");
  }

  free(shaderCode);

  return shaderModule;
}

Pipeline pipeline_create(const char *vertexShader, const char *fragmentShader) {
  if (pipeline_cache == VK_NULL_HANDLE) {
    load_pipeline_cache();
  }

  VkDevice device = renderer_get_device();

  Pipeline pipeline = {0};

  pipeline.vertex_module = compile_shader(vertexShader);
  pipeline.fragment_module = compile_shader(fragmentShader);

  vulkan_pipeline_create(&pipeline);

  return pipeline;
}

void vulkan_pipeline_create(Pipeline *pipeline) {
  VkDevice device = renderer_get_device();

  VkPipelineShaderStageCreateInfo shaderStages[2] = {{0}, {0}};
  shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].pNext = NULL;
  shaderStages[0].flags = 0;
  shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module = pipeline->vertex_module;
  shaderStages[0].pName = "main";
  shaderStages[0].pSpecializationInfo = NULL;

  shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].pNext = NULL;
  shaderStages[1].flags = 0;
  shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = pipeline->fragment_module;
  shaderStages[1].pName = "main";
  shaderStages[1].pSpecializationInfo = NULL;

  VkVertexInputBindingDescription vertexBindingDescription = {0};
  vertexBindingDescription.binding = 0;
  vertexBindingDescription.stride = 4 * sizeof(float);
  vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  // in vec4 inPos;
  VkVertexInputAttributeDescription vertexAttributeDescription = {0};
  vertexAttributeDescription.location = 0;
  vertexAttributeDescription.binding = 0;
  vertexAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  vertexAttributeDescription.offset = 0;

  VkPipelineVertexInputStateCreateInfo vertexInput = {0};
  vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInput.pNext = NULL;
  vertexInput.flags = 0;
  vertexInput.vertexBindingDescriptionCount = 1;
  vertexInput.pVertexBindingDescriptions = &vertexBindingDescription;
  vertexInput.vertexAttributeDescriptionCount = 1;
  vertexInput.pVertexAttributeDescriptions = &vertexAttributeDescription;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.pNext = NULL;
  inputAssembly.flags = 0;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

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

  VkPipelineViewportStateCreateInfo viewportInfo = {0};
  viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportInfo.pNext = NULL;
  viewportInfo.flags = 0;
  viewportInfo.viewportCount = 1;
  viewportInfo.pViewports = &viewport;
  viewportInfo.scissorCount = 1;
  viewportInfo.pScissors = &scissor;

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

  VkPipelineDepthStencilStateCreateInfo depthStencil = {0};
  depthStencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.pNext = NULL;
  depthStencil.flags = 0;
  depthStencil.depthTestEnable = VK_FALSE;
  depthStencil.depthWriteEnable = VK_FALSE;
  depthStencil.depthCompareOp = 0;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.depthTestEnable = VK_FALSE;
  depthStencil.depthTestEnable = VK_FALSE;
  depthStencil.depthTestEnable = VK_FALSE;
  depthStencil.minDepthBounds = 0.f;
  depthStencil.maxDepthBounds = 1.f;

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = 0.0f;
  colorBlendAttachment.dstAlphaBlendFactor = 0.0f;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = 0.0f;
  colorBlendAttachment.dstAlphaBlendFactor = 0.0f;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlend = {0};
  colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlend.pNext = NULL;
  colorBlend.flags = 0;
  colorBlend.logicOpEnable = VK_FALSE;
  colorBlend.logicOp = VK_LOGIC_OP_COPY;
  colorBlend.attachmentCount = 1;
  colorBlend.pAttachments = &colorBlendAttachment;

  // Uniform color
  VkPushConstantRange pushConstantRange = {0};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = 3 * sizeof(float);

  VkPipelineLayoutCreateInfo layoutCreateInfo = {0};
  layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutCreateInfo.pNext = NULL;
  layoutCreateInfo.flags = 0;
  layoutCreateInfo.setLayoutCount = 0;
  layoutCreateInfo.pSetLayouts = NULL;
  layoutCreateInfo.pushConstantRangeCount = 1;
  layoutCreateInfo.pPushConstantRanges = &pushConstantRange;

  if (vkCreatePipelineLayout(device, &layoutCreateInfo, NULL,
                             &pipeline->pipeline_layout) != VK_SUCCESS) {
    window_fail_with_error(
        "An error occured while creating the pipeline layout.");
  }

  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {0};
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.pNext = NULL;
  pipelineCreateInfo.flags = 0;
  pipelineCreateInfo.stageCount = 2;
  pipelineCreateInfo.pStages = shaderStages;
  pipelineCreateInfo.pVertexInputState = &vertexInput;
  pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
  pipelineCreateInfo.pTessellationState = NULL;
  pipelineCreateInfo.pViewportState = &viewportInfo;
  pipelineCreateInfo.pRasterizationState = &rasterization;
  pipelineCreateInfo.pMultisampleState = &multisample;
  pipelineCreateInfo.pDepthStencilState = &depthStencil;
  pipelineCreateInfo.pColorBlendState = &colorBlend;
  pipelineCreateInfo.pDynamicState = NULL;
  pipelineCreateInfo.layout = pipeline->pipeline_layout;
  pipelineCreateInfo.renderPass = renderer_get_render_pass();
  pipelineCreateInfo.subpass = 0;
  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCreateInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(device, pipeline_cache, 1, &pipelineCreateInfo,
                                NULL, &pipeline->pipeline) != VK_SUCCESS) {
    window_fail_with_error(
        "An error occured while creating the graphics pipeline.");
  }

  write_pipeline_cache();
}

void pipeline_destroy(Pipeline pipeline) {
  VkDevice device = renderer_get_device();

  vkDestroyShaderModule(device, pipeline.vertex_module, NULL);
  vkDestroyShaderModule(device, pipeline.fragment_module, NULL);

  vulkan_pipeline_destroy(&pipeline);
}

void vulkan_pipeline_destroy(Pipeline *pipeline) {
  VkDevice device = renderer_get_device();

  vkDestroyPipelineLayout(device, pipeline->pipeline_layout, NULL);
  vkDestroyPipeline(device, pipeline->pipeline, NULL);

  pipeline->pipeline_layout = VK_NULL_HANDLE;
  pipeline->pipeline = VK_NULL_HANDLE;
}
