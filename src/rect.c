#include "rect.h"
#include <vulkan/vulkan.h>

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

static VkBuffer vertex_buffer = VK_NULL_HANDLE;

static VkDeviceMemory vertex_buffer_memory = VK_NULL_HANDLE;

void rect_init() {
  vertex_shader = shader_create("shaders/rect.vert.spv");
  geometry_shader = shader_create("shaders/rect.geom.spv");
  fragment_shader = shader_create("shaders/rect.frag.spv");

  pipeline_add_shader(&pipeline, vertex_shader, VK_SHADER_STAGE_VERTEX_BIT);
  pipeline_add_shader(&pipeline, geometry_shader, VK_SHADER_STAGE_GEOMETRY_BIT);
  pipeline_add_shader(&pipeline, fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);

  // in vec2 in_pos;
  pipeline_add_attribute(&pipeline, VK_FORMAT_R32G32_SFLOAT, 0);
  pipeline_add_attribute(&pipeline, VK_FORMAT_R32G32_SFLOAT, 2 * sizeof(float));

  pipeline_add_push_constant(&pipeline, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                             3 * sizeof(float));

  pipeline_create(&pipeline);

  renderer_create_buffer(vertices_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         &vertex_buffer, &vertex_buffer_memory);

  uint32_t command_buffer_count = 0;
  VkCommandBuffer *command_buffers =
      renderer_begin_command_buffers(&command_buffer_count);

  for (uint32_t i = 0; i < command_buffer_count; i++) {
    vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline.pipeline);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(command_buffers[i], 0, 1, &vertex_buffer, &offset);

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

  vkDestroyBuffer(device, vertex_buffer, NULL);
  vkFreeMemory(device, vertex_buffer_memory, NULL);

  shader_destroy(vertex_shader);
  shader_destroy(geometry_shader);
  shader_destroy(fragment_shader);

  pipeline_destroy(pipeline);
}

void rect_draw() {
  renderer_buffer_data(vertex_buffer_memory, vertices_size,
                       rect_get_vertices());
}

Rect *rect_new() { return &vertices[count++]; }

Rect *rect_get_vertices() { return vertices; }

int rect_intersect(Rect *r1, Rect *r2) {
  return (r1->x < r2->x + r2->w && r2->x < r1->x + r1->w &&
          r1->y < r2->y + r2->h && r2->y < r1->y + r1->h);
}
