#ifndef FLAP_RENDERER_H_
#define FLAP_RENDERER_H_

#include <vulkan/vulkan.h>

#include "pipeline.h"

/**
 * A Vulkan renderer holds all global Vulkan objects.
 */

void renderer_init();

void renderer_quit();

void renderer_render();

void renderer_create_swapchain();
void renderer_cleanup_swapchain();

VkDevice renderer_get_device();

VkPhysicalDeviceProperties renderer_get_physical_device_properties();

VkPhysicalDeviceMemoryProperties
renderer_get_physical_device_memory_properties();

uint32_t renderer_get_image_count();

const VkExtent2D renderer_get_extent();

VkRenderPass renderer_get_render_pass();

void renderer_set_pipeline(Pipeline *pipeline);

VkCommandBuffer *renderer_begin_command_buffer();

void renderer_end_command_buffer(VkCommandBuffer *command_buffer);

VkCommandBuffer *renderer_begin_command_buffers(uint32_t *buffer_count);

void renderer_end_command_buffers();

#endif // FLAP_RENDERER_H_
