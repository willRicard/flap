#include "vk/pipeline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "vk/renderer.h"

static VkPipelineCache pipelineCache = VK_NULL_HANDLE;

static void loadPipelineCache() {
  FILE *cacheFile = NULL;
  long size = 0;
  char *cacheData = NULL;

  cacheFile = fopen("pipeline_cache.bin", "r");

  if (cacheFile != NULL) {
    fseek(cacheFile, 0, SEEK_END);

    size = ftell(cacheFile);

    cacheData = (char *)malloc(size * sizeof(char));

    rewind(cacheFile);

    fread(cacheData, size, 1, cacheFile);

    fclose(cacheFile);
  } else {
    fputs("Error opening pipeline cache.", stderr);
  }

  VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
  pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  pipelineCacheCreateInfo.initialDataSize = size;
  pipelineCacheCreateInfo.pInitialData = cacheData;

  if (vkCreatePipelineCache(flapRendererGetDevice(), &pipelineCacheCreateInfo,
                            NULL, &pipelineCache) != VK_SUCCESS) {
    fputs("Error creating pipeline cache.", stderr);
  }

  if (cacheData != NULL) {
    free(cacheData);
  }
}

static void writePipelineCache() {
  FILE *cacheFile = NULL;
  cacheFile = fopen("pipeline_cache.bin", "w");
  if (cacheFile == NULL) {
    fputs("Error writing pipeline cache.", stderr);
    return;
  }

  size_t size;

  vkGetPipelineCacheData(flapRendererGetDevice(), pipelineCache, &size, NULL);

  char *data = (char *)malloc(size * sizeof(char));

  vkGetPipelineCacheData(flapRendererGetDevice(), pipelineCache, &size, data);

  fwrite(data, size, 1, cacheFile);

  fclose(cacheFile);
}

static VkShaderModule compileShader(const unsigned int codeSize,
                                    const unsigned char *shaderSource) {
  VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
  shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderModuleCreateInfo.codeSize = codeSize;
  shaderModuleCreateInfo.pCode = (const uint32_t *)shaderSource;

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(flapRendererGetDevice(), &shaderModuleCreateInfo,
                           NULL, &shaderModule) != VK_SUCCESS) {
    fputs("Error while creating the vertex shader module.", stderr);
    exit(EXIT_FAILURE);
  }
  return shaderModule;
}

flapPipeline flapPipelineCreate(const unsigned int vertexCodeSize,
                                const unsigned char *vertexShaderSource,
                                const unsigned int fragmentCodeSize,
                                const unsigned char *fragmentShaderSource) {
  if (pipelineCache == VK_NULL_HANDLE) {
    loadPipelineCache();
  }

  VkDevice device = flapRendererGetDevice();

  flapPipeline pipeline = {};

  pipeline.vertexModule = compileShader(vertexCodeSize, vertexShaderSource);
  pipeline.fragmentModule =
      compileShader(fragmentCodeSize, fragmentShaderSource);

  VkPipelineShaderStageCreateInfo shaderStages[2] = {};
  shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module = pipeline.vertexModule;
  ;
  shaderStages[0].pName = "main";
  shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = pipeline.fragmentModule;
  shaderStages[1].pName = "main";

  VkVertexInputBindingDescription vertexBindingDescription = {};
  vertexBindingDescription.binding = 0;
  vertexBindingDescription.stride = 0;
  vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  // in vec3 inPos;
  VkVertexInputAttributeDescription vertexAttributeDescription = {};
  vertexAttributeDescription.location = 0;
  vertexAttributeDescription.binding = 0;
  vertexAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  vertexAttributeDescription.offset = 0;

  VkPipelineVertexInputStateCreateInfo vertexInput = {};
  vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInput.vertexBindingDescriptionCount = 1;
  vertexInput.pVertexBindingDescriptions = &vertexBindingDescription;
  vertexInput.vertexAttributeDescriptionCount = 1;
  vertexInput.pVertexAttributeDescriptions = &vertexAttributeDescription;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport = {};
  viewport.maxDepth = 1.0f;
  viewport.width = (float)flapRendererGetExtent().width;
  viewport.height = (float)flapRendererGetExtent().height;

  VkRect2D scissor;
  scissor.extent = flapRendererGetExtent();
  scissor.offset.x = 0;
  scissor.offset.y = 0;

  VkPipelineViewportStateCreateInfo viewportInfo = {};
  viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportInfo.viewportCount = 1;
  viewportInfo.pViewports = &viewport;
  viewportInfo.scissorCount = 1;
  viewportInfo.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterization = {};
  rasterization.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterization.depthClampEnable = VK_FALSE;
  rasterization.rasterizerDiscardEnable = VK_FALSE;
  rasterization.polygonMode = VK_POLYGON_MODE_FILL;
  rasterization.cullMode = VK_CULL_MODE_NONE;
  rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterization.depthBiasEnable = VK_FALSE;
  rasterization.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisample = {};
  multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisample.sampleShadingEnable = VK_FALSE;

  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_FALSE;
  depthStencil.maxDepthBounds = 1.f;

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlend = {};
  colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlend.logicOpEnable = VK_FALSE;
  colorBlend.attachmentCount = 1;
  colorBlend.pAttachments = &colorBlendAttachment;

  // Uniform color
  VkPushConstantRange pushConstantRange = {};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = 3 * sizeof(float);

  VkPipelineLayoutCreateInfo layoutCreateInfo = {};
  layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutCreateInfo.setLayoutCount = 0;
  layoutCreateInfo.pSetLayouts = NULL;
  layoutCreateInfo.pushConstantRangeCount = 1;
  layoutCreateInfo.pPushConstantRanges = &pushConstantRange;

  if (vkCreatePipelineLayout(device, &layoutCreateInfo, NULL,
                             &pipeline.pipelineLayout) != VK_SUCCESS) {
    fputs("An error occured while creating the pipeline layout.", stderr);
    exit(EXIT_FAILURE);
  }

  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.stageCount = 2;
  pipelineCreateInfo.pStages = shaderStages;
  pipelineCreateInfo.pVertexInputState = &vertexInput;
  pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
  pipelineCreateInfo.pViewportState = &viewportInfo;
  pipelineCreateInfo.pRasterizationState = &rasterization;
  pipelineCreateInfo.pMultisampleState = &multisample;
  pipelineCreateInfo.pDepthStencilState = &depthStencil;
  pipelineCreateInfo.pColorBlendState = &colorBlend;
  pipelineCreateInfo.layout = pipeline.pipelineLayout;
  pipelineCreateInfo.renderPass = flapRendererGetRenderPass();
  pipelineCreateInfo.subpass = 0;
  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCreateInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo,
                                NULL, &pipeline.pipeline) != VK_SUCCESS) {
    fputs("An error occured while creating the graphics pipeline.", NULL);
    exit(EXIT_FAILURE);
  }

  writePipelineCache();

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
