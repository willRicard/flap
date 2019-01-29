#include "rect.h"

#include <vulkan/vulkan.h>

#include "buffer.h"
#include "device.h"
#include "error.h"
#include "flap.h"
#include "shader.h"
#include "swapchain.h"

const float FLAP_BIRD_COLOR[] = {1.f, 1.f, 0.f};
const float FLAP_PIPE_COLOR[] = {0.035f, 0.42f, 0.035f};

static unsigned int rect_count = 0;

static Rect rect_vertices[1 + FLAP_NUM_PIPES * 2] = {{0}};

static Shader rect_shaders[3] = {{0}};

static Buffer rect_vertex_buffer = {0};

static VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

void rect_init(Device *dev) {
  // Load shaders
  shader_create(dev, "shaders/rect.vert.spv", VK_SHADER_STAGE_VERTEX_BIT,
                &rect_shaders[0]);
  shader_create(dev, "shaders/rect.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT,
                &rect_shaders[1]);
  shader_create(dev, "shaders/rect.geom.spv", VK_SHADER_STAGE_GEOMETRY_BIT,
                &rect_shaders[2]);

  // Create vertex buffer
  buffer_create(dev, sizeof(rect_vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &rect_vertex_buffer);

  // Add push constant ranges
  VkPushConstantRange push_constant_range = {0};
  push_constant_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  push_constant_range.offset = 0;
  push_constant_range.size = 3 * sizeof(float);

  VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.pushConstantRangeCount = 1;
  pipeline_layout_info.pPushConstantRanges = &push_constant_range;

  error_check(vkCreatePipelineLayout(dev->device, &pipeline_layout_info, NULL,
                                     &pipeline_layout),
              "vkCreatePipelineLayout");
}

void rect_quit(Device *dev) {
  vkDeviceWaitIdle(dev->device);

  vkDestroyPipelineLayout(dev->device, pipeline_layout, NULL);

  buffer_destroy(dev, &rect_vertex_buffer);

  for (uint32_t i = 0; i < 3; i++) {
    shader_destroy(dev, &rect_shaders[i]);
  }
}

void rect_get_pipeline_create_info(
    VkGraphicsPipelineCreateInfo *pipeline_info) {

  // Add shader stages
  static VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
  input_assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;
  pipeline_info->pInputAssemblyState = &input_assembly;

  pipeline_info->stageCount = 3;
  pipeline_info->pStages = rect_shaders;

  // Add vertex attributes
  static VkVertexInputBindingDescription rect_vertex_binding = {0};
  rect_vertex_binding.binding = 0;
  rect_vertex_binding.stride = 4 * sizeof(float);
  rect_vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  static VkVertexInputAttributeDescription rect_vertex_attributes[2] = {{0}};
  rect_vertex_attributes[0].location = 0;
  rect_vertex_attributes[0].binding = 0;
  rect_vertex_attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
  rect_vertex_attributes[0].offset = 0;

  rect_vertex_attributes[1].location = 1;
  rect_vertex_attributes[1].binding = 0;
  rect_vertex_attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
  rect_vertex_attributes[1].offset = 2 * sizeof(float);

  static VkPipelineVertexInputStateCreateInfo rect_vertex_input_info = {0};
  rect_vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  rect_vertex_input_info.vertexBindingDescriptionCount = 1;
  rect_vertex_input_info.pVertexBindingDescriptions = &rect_vertex_binding;
  rect_vertex_input_info.vertexAttributeDescriptionCount = 2;
  rect_vertex_input_info.pVertexAttributeDescriptions = rect_vertex_attributes;

  pipeline_info->pVertexInputState = &rect_vertex_input_info;

  pipeline_info->layout = pipeline_layout;
}

void rect_record_command_buffer(VkCommandBuffer cmd_buf) {
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(cmd_buf, 0, 1, &rect_vertex_buffer.buffer, &offset);

  // Draw player
  vkCmdPushConstants(cmd_buf, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                     3 * sizeof(float), FLAP_BIRD_COLOR);
  vkCmdDraw(cmd_buf, 1, 1, 0, 0);

  // Draw pipes
  vkCmdPushConstants(cmd_buf, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                     3 * sizeof(float), FLAP_PIPE_COLOR);
  vkCmdDraw(cmd_buf, FLAP_NUM_PIPES * 2, 1, 1, 0);
}

void rect_update() { buffer_write(rect_vertices, &rect_vertex_buffer); }

Rect *rect_new() { return &rect_vertices[rect_count++]; }

Rect *rect_get_vertices() { return rect_vertices; }

int rect_intersect(Rect *r1, Rect *r2) {
  return (r1->x < r2->x + r2->w && r2->x < r1->x + r1->w &&
          r1->y < r2->y + r2->h && r2->y < r1->y + r1->h);
}
