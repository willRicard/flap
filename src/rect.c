#include "rect.h"
#include <vulkan/vulkan.h>

#include "buffer.h"
#include "flap.h"
#include "pipeline.h"
#include "renderer.h"
#include "shader.h"

const float FLAP_BIRD_COLOR[] = {1.f, 1.f, 0.f};
const float FLAP_PIPE_COLOR[] = {0.035f, 0.42f, 0.035f};

static unsigned int count = 0;

static Rect vertices[1 + FLAP_NUM_PIPES * 2];

static const VkDeviceSize vertices_size =
    (1 + FLAP_NUM_PIPES * 2) * sizeof(Rect);

static VkShaderModule vertex_shader = VK_NULL_HANDLE;
static VkShaderModule geometry_shader = VK_NULL_HANDLE;
static VkShaderModule fragment_shader = VK_NULL_HANDLE;

static Pipeline pipeline = {0};

static Buffer vertex_buffer = {0};

void rect_init() {
  vertex_shader = shader_create("shaders/rect.vert.spv");
  geometry_shader = shader_create("shaders/rect.geom.spv");
  fragment_shader = shader_create("shaders/rect.frag.spv");

  VkPipelineShaderStageCreateInfo vertex_stage_info = {0};
  vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertex_stage_info.pNext = NULL;
  vertex_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertex_stage_info.pName = "main";
  vertex_stage_info.module = vertex_shader;

  VkPipelineShaderStageCreateInfo fragment_stage_info = {0};
  fragment_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragment_stage_info.pNext = NULL;
  fragment_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragment_stage_info.pName = "main";
  fragment_stage_info.module = fragment_shader;

  VkPipelineShaderStageCreateInfo geometry_stage_info = {0};
  geometry_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  geometry_stage_info.pNext = NULL;
  geometry_stage_info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
  geometry_stage_info.pName = "main";
  geometry_stage_info.module = geometry_shader;

  VkVertexInputBindingDescription vertex_binding = {0};
  vertex_binding.binding = 0;
  vertex_binding.stride = 4 * sizeof(float);
  vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputAttributeDescription vertex_pos_attr = {0};
  vertex_pos_attr.location = 0;
  vertex_pos_attr.binding = 0;
  vertex_pos_attr.format = VK_FORMAT_R32G32_SFLOAT;
  vertex_pos_attr.offset = 0;

  VkVertexInputAttributeDescription vertex_size_attr = {0};
  vertex_size_attr.location = 1;
  vertex_size_attr.binding = 0;
  vertex_size_attr.format = VK_FORMAT_R32G32_SFLOAT;
  vertex_size_attr.offset = 2 * sizeof(float);

  VkPushConstantRange push_constant_range = {0};
  push_constant_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  push_constant_range.offset = 0;
  push_constant_range.size = 3 * sizeof(float);

  VkPipelineShaderStageCreateInfo shader_stages[] = {
      vertex_stage_info, fragment_stage_info, geometry_stage_info};

  VkVertexInputAttributeDescription attributes[] = {vertex_pos_attr,
                                                    vertex_size_attr};

  PipelineCreateInfo pipeline_info = {0};
  pipeline_info.num_shader_stages = 3;
  pipeline_info.shader_stages = shader_stages;
  pipeline_info.num_bindings = 1;
  pipeline_info.bindings = &vertex_binding;
  pipeline_info.num_attributes = 2;
  pipeline_info.attributes = attributes;
  pipeline_info.num_push_constant_ranges = 1;
  pipeline_info.push_constant_ranges = &push_constant_range;

  pipeline = pipeline_create(&pipeline_info);

  buffer_create(&vertex_buffer, vertices_size,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, BUFFER_TYPE_HOST_BUFFER);

  uint32_t command_buffer_count = 0;
  VkCommandBuffer *command_buffers =
      renderer_begin_command_buffers(&command_buffer_count);

  for (uint32_t i = 0; i < command_buffer_count; i++) {
    vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline.pipeline);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(command_buffers[i], 0, 1, &vertex_buffer.buffer,
                           &offset);

    // Draw player
    vkCmdPushConstants(command_buffers[i], pipeline.pipeline_layout,
                       VK_SHADER_STAGE_FRAGMENT_BIT, 0, 3 * sizeof(float),
                       FLAP_BIRD_COLOR);
    vkCmdDraw(command_buffers[i], 1, 1, 0, 0);

    // Draw pipes
    vkCmdPushConstants(command_buffers[i], pipeline.pipeline_layout,
                       VK_SHADER_STAGE_FRAGMENT_BIT, 0, 3 * sizeof(float),
                       FLAP_PIPE_COLOR);
    vkCmdDraw(command_buffers[i], FLAP_NUM_PIPES * 2, 1, 1, 0);
  }

  renderer_end_command_buffers();
}

void rect_quit() {
  VkDevice device = renderer_get_device();
  vkDeviceWaitIdle(device);

  buffer_destroy(&vertex_buffer);

  shader_destroy(vertex_shader);
  shader_destroy(geometry_shader);
  shader_destroy(fragment_shader);

  pipeline_destroy(pipeline);
}

void rect_draw() { buffer_write(vertex_buffer, vertices_size, vertices); }

Rect *rect_new() { return &vertices[count++]; }

Rect *rect_get_vertices() { return vertices; }

int rect_intersect(Rect *r1, Rect *r2) {
  return (r1->x < r2->x + r2->w && r2->x < r1->x + r1->w &&
          r1->y < r2->y + r2->h && r2->y < r1->y + r1->h);
}
