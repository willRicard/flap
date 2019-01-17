#include "rect.h"
#include <vulkan/vulkan.h>

#include "buffer.h"
#include "flap.h"
#include "pipeline.h"
#include "renderer.h"

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

static VkDeviceMemory vertex_buffer_memory = VK_NULL_HANDLE;

void rect_init() {
  vertex_shader = shader_create("shaders/rect.vert.spv");
  geometry_shader = shader_create("shaders/rect.geom.spv");
  fragment_shader = shader_create("shaders/rect.frag.spv");

  pipeline_add_shader(&pipeline, vertex_shader, VK_SHADER_STAGE_VERTEX_BIT);
  pipeline_add_shader(&pipeline, geometry_shader, VK_SHADER_STAGE_GEOMETRY_BIT);
  pipeline_add_shader(&pipeline, fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);

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

  VkVertexInputAttributeDescription attributes[] = {vertex_pos_attr,
                                                    vertex_size_attr};
  pipeline_add_attributes(&pipeline, 1, &vertex_binding, 2, attributes);

  pipeline_add_push_constant(&pipeline, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                             3 * sizeof(float));

  pipeline_create(&pipeline);

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
