#ifndef FLAP_RENDERER_H_
#define FLAP_RENDERER_H_

#include <vulkan/vulkan.h>

#include "pipeline_vk.h"

/**
 * A Vulkan renderer holds all global Vulkan objects.
 */

void renderer_init();

void renderer_quit();

void renderer_render();

void renderer_create_swapchain();
void renderer_cleanup_swapchain();

VkDevice renderer_get_device();

const VkExtent2D renderer_get_extent();

const VkRenderPass renderer_get_render_pass();

void renderer_set_pipeline(Pipeline *pipeline);

void renderer_set_vertex_buffer(VkBuffer buffer);

void renderer_set_index_buffer(VkBuffer buffer);

void renderer_record_command_buffers();

void renderer_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags memory_properties,
                            VkBuffer *buffer, VkDeviceMemory *buffer_memory);

void renderer_buffer_data(VkDeviceMemory bufferMemory, VkDeviceSize size,
                          const void *data);

#endif // FLAP_RENDERER_H_
