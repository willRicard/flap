#include "pipeline_vk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "assets.h"
#include "renderer_vk.h"
#include "window.h"

static VkPipelineCache pipelineCache = VK_NULL_HANDLE;

static void loadPipelineCache() {
  uint32_t *initialData = NULL;
  size_t initialDataSize = 0;
  initialData =
      (uint32_t *)flapAssetsReadFile("pipeline_cache.bin", &initialDataSize);

  VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {0};
  pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  pipelineCacheCreateInfo.pNext = NULL;
  pipelineCacheCreateInfo.flags = 0;
  pipelineCacheCreateInfo.initialDataSize = initialDataSize;
  pipelineCacheCreateInfo.pInitialData = initialData;

  if (vkCreatePipelineCache(flapRendererGetDevice(), &pipelineCacheCreateInfo,
                            NULL, &pipelineCache) != VK_SUCCESS) {
    flapWindowFailWithError("Error creating pipeline cache.");
  }
  free(initialData);
}

static void writePipelineCache() {
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

  vkGetPipelineCacheData(flapRendererGetDevice(), pipelineCache, &size, NULL);

  char *data = (char *)malloc(size * sizeof(char));

  vkGetPipelineCacheData(flapRendererGetDevice(), pipelineCache, &size, data);

  fwrite(data, size, 1, cacheFile);

  fclose(cacheFile);
}

static VkShaderModule compileShader(const char *shaderFile) {

  VkShaderModuleCreateInfo shaderModuleCreateInfo = {0};
  shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderModuleCreateInfo.pNext = NULL;
  shaderModuleCreateInfo.flags = 0;

  uint32_t *shaderCode = NULL;
  shaderCode = (uint32_t *)flapAssetsReadFile(shaderFile,
                                              &shaderModuleCreateInfo.codeSize);
  if (shaderCode == NULL) {
    flapWindowFailWithError("Failed reading shader code.");
  }
  shaderModuleCreateInfo.pCode = (const uint32_t *)shaderCode;

  VkShaderModule shaderModule = VK_NULL_HANDLE;
  if (vkCreateShaderModule(flapRendererGetDevice(), &shaderModuleCreateInfo,
                           NULL, &shaderModule) != VK_SUCCESS) {
    flapWindowFailWithError("Error while creating the vertex shader module.");
  }

  free(shaderCode);

  return shaderModule;
}

flapPipeline flapPipelineCreate(const char *vertexShader,
                                const char *fragmentShader) {

  if (pipelineCache == VK_NULL_HANDLE) {
    loadPipelineCache();
  }

  VkDevice device = flapRendererGetDevice();

  flapPipeline pipeline = {0};

  pipeline.vertexModule = compileShader(vertexShader);
  pipeline.fragmentModule = compileShader(fragmentShader);

  VkPipelineShaderStageCreateInfo shaderStages[2] = {{0}, {0}};
  shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].pNext = NULL;
  shaderStages[0].flags = 0;
  shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module = pipeline.vertexModule;
  shaderStages[0].pName = "main";
  shaderStages[0].pSpecializationInfo = NULL;

  shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].pNext = NULL;
  shaderStages[1].flags = 0;
  shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = pipeline.fragmentModule;
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
  // vertexAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
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
  viewport.width = (float)flapRendererGetExtent().width;
  viewport.height = (float)flapRendererGetExtent().height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {0};
  scissor.extent = flapRendererGetExtent();
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
                             &pipeline.pipelineLayout) != VK_SUCCESS) {
    flapWindowFailWithError(
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
  pipelineCreateInfo.layout = pipeline.pipelineLayout;
  pipelineCreateInfo.renderPass = flapRendererGetRenderPass();
  pipelineCreateInfo.subpass = 0;
  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCreateInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo,
                                NULL, &pipeline.pipeline) != VK_SUCCESS) {
    flapWindowFailWithError(
        "An error occured while creating the graphics pipeline.");
  }

  // writePipelineCache();

  return pipeline;
}

void flapPipelineDestroy(flapPipeline pipeline) {
  VkDevice device = flapRendererGetDevice();
  vkDestroyShaderModule(device, pipeline.vertexModule, NULL);
  vkDestroyShaderModule(device, pipeline.fragmentModule, NULL);
  vkDestroyPipelineLayout(device, pipeline.pipelineLayout, NULL);
  vkDestroyPipeline(device, pipeline.pipeline, NULL);

  vkDestroyPipelineCache(device, pipelineCache, NULL);
}
