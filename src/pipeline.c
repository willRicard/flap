#include "pipeline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "assets.h"
#include "renderer.h"
#include "window.h"

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

VkShaderModule shader_create(const char *source_file) {
  VkShaderModuleCreateInfo module_info = {0};
  module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  module_info.pNext = NULL;
  module_info.flags = 0;

  uint32_t *shader_code = NULL;
  shader_code =
      (uint32_t *)assets_read_file(source_file, &module_info.codeSize);
  if (shader_code == NULL) {
    window_fail_with_error("Failed reading shader code.");
  }
  module_info.pCode = (const uint32_t *)shader_code;

  VkShaderModule module = VK_NULL_HANDLE;
  if (vkCreateShaderModule(renderer_get_device(), &module_info, NULL,
                           &module) != VK_SUCCESS) {
    window_fail_with_error("Error while creating the vertex shader module.");
  }

  free(shader_code);

  return module;
}

void shader_destroy(VkShaderModule module) {
  vkDestroyShaderModule(renderer_get_device(), module, NULL);
}

void pipeline_add_shader(Pipeline *pipeline, VkShaderModule shader_module,
                         VkShaderStageFlags shader_stage) {
  VkPipelineShaderStageCreateInfo *shader_info =
      &pipeline->shader_stages[pipeline->num_shader_stages++];

  shader_info->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_info->pNext = NULL;
  shader_info->flags = 0;
  shader_info->stage = shader_stage;
  shader_info->module = shader_module;
  shader_info->pName = "main";
  shader_info->pSpecializationInfo = NULL;
}

void pipeline_add_attributes(Pipeline *pipeline, uint32_t num_bindings,
                             VkVertexInputBindingDescription *bindings,
                             uint32_t num_attributes,
                             VkVertexInputAttributeDescription *attributes) {
  pipeline->num_attributes = num_attributes;
  pipeline->num_bindings = num_bindings;
  pipeline->bindings = bindings;
  pipeline->attributes = attributes;
}

void pipeline_add_push_constant(Pipeline *pipeline,
                                VkShaderStageFlags shader_stage,
                                VkDeviceSize offset, VkDeviceSize size) {
  VkPushConstantRange *push_constant_range =
      &pipeline->push_constants[pipeline->num_push_constants++];
  push_constant_range->stageFlags = shader_stage;
  push_constant_range->offset = offset;
  push_constant_range->size = size;
}

void pipeline_add_set_layout(Pipeline *pipeline,
                             VkDescriptorSetLayout set_layout) {
  pipeline->set_layouts[pipeline->num_set_layouts++] = set_layout;
}

void pipeline_create(Pipeline *pipeline) {
  VkDevice device = renderer_get_device();

  VkVertexInputBindingDescription binding_description = {0};
  binding_description.binding = 0;
  binding_description.stride = pipeline->vertex_attribute_stride;
  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputBindingDescription instance_description = {0};
  instance_description.binding = 0;
  instance_description.stride = pipeline->instance_attribute_stride;
  instance_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

  VkPipelineVertexInputStateCreateInfo vertexInput = {0};
  vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  vertexInput.vertexBindingDescriptionCount = pipeline->num_bindings;
  vertexInput.pVertexBindingDescriptions = pipeline->bindings;

  vertexInput.vertexAttributeDescriptionCount = pipeline->num_attributes;
  vertexInput.pVertexAttributeDescriptions = pipeline->attributes;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.pNext = NULL;
  inputAssembly.flags = 0;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
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

  VkPipelineLayoutCreateInfo layoutCreateInfo = {0};
  layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutCreateInfo.pNext = NULL;
  layoutCreateInfo.flags = 0;
  layoutCreateInfo.setLayoutCount = pipeline->num_set_layouts;
  layoutCreateInfo.pSetLayouts = pipeline->set_layouts;
  layoutCreateInfo.pushConstantRangeCount = pipeline->num_push_constants;
  layoutCreateInfo.pPushConstantRanges = pipeline->push_constants;

  if (vkCreatePipelineLayout(device, &layoutCreateInfo, NULL,
                             &pipeline->pipeline_layout) != VK_SUCCESS) {
    window_fail_with_error(
        "An error occured while creating the pipeline layout.");
  }

  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {0};
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.pNext = NULL;
  pipelineCreateInfo.flags = 0;

  pipelineCreateInfo.stageCount = pipeline->num_shader_stages;
  pipelineCreateInfo.pStages = pipeline->shader_stages,

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
}

void pipeline_destroy(Pipeline pipeline) {
  VkDevice device = renderer_get_device();

  vkDestroyPipelineLayout(device, pipeline.pipeline_layout, NULL);
  vkDestroyPipeline(device, pipeline.pipeline, NULL);
}
