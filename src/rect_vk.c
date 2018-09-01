#include "rect.h"

#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include "flap.h"

#include "pipeline_vk.h"
#include "renderer_vk.h"

static Pipeline pipeline;

static VkBuffer vertex_buffer = VK_NULL_HANDLE;
static VkBuffer index_buffer = VK_NULL_HANDLE;

static VkDeviceMemory vertex_buffer_memory = VK_NULL_HANDLE;
static VkDeviceMemory index_buffer_memory = VK_NULL_HANDLE;

static const VkDeviceSize vertices_size =
    (1 + FLAP_NUM_PIPES * 2) * sizeof(Rect);
static const VkDeviceSize indices_size =
    (1 + FLAP_NUM_PIPES * 2) * 6 * sizeof(uint16_t);

void rect_init() {
  pipeline = pipeline_create("shaders/rect.vert.spv", "shaders/rect.frag.spv");

  renderer_create_buffer(vertices_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         &vertex_buffer, &vertex_buffer_memory);

  renderer_create_buffer(indices_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         &index_buffer, &index_buffer_memory);

  renderer_set_pipeline(&pipeline);
  renderer_set_vertex_buffer(vertex_buffer);
  renderer_set_index_buffer(index_buffer);

  renderer_record_command_buffers();
}

void rect_quit() {
  VkDevice device = renderer_get_device();
  vkDeviceWaitIdle(device);

  vkDestroyBuffer(device, vertex_buffer, NULL);
  vkFreeMemory(device, vertex_buffer_memory, NULL);

  vkDestroyBuffer(device, index_buffer, NULL);
  vkFreeMemory(device, index_buffer_memory, NULL);

  pipeline_destroy(pipeline);
}

void rect_draw() {
  renderer_buffer_data(vertex_buffer_memory, vertices_size,
                       rect_get_vertices());
  renderer_buffer_data(index_buffer_memory, indices_size, rect_get_indices());
}
