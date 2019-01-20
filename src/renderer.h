#ifndef FLAP_RENDERER_H_
#define FLAP_RENDERER_H_

#include <vulkan/vulkan.h>

#include "pipeline.h"

/**
 * Global interface to the VkDevice.
 */

/**
 * Setup the VkDevice and window swapchain.
 */
void renderer_init();

/**
 * Clear all Vulkan resources.
 */
void renderer_quit();

/**
 * Execute command buffers and present the window.
 */
void renderer_render();

/**
 * Dynamic swapchain recreation.
 */
void renderer_create_swapchain();

/**
 * Clean swapchain resources.
 */
void renderer_cleanup_swapchain();

/**
 * Return the global handle to the VkDevice.
 */
VkDevice renderer_get_device();

VkPhysicalDeviceProperties renderer_get_physical_device_properties();

VkPhysicalDeviceMemoryProperties
renderer_get_physical_device_memory_properties();

/**
 * Get multiple buffering image count.
 */
uint32_t renderer_get_image_count();

/**
 * Get the render surface extent.
 */
const VkExtent2D renderer_get_extent();

VkRenderPass renderer_get_render_pass();

VkCommandBuffer *renderer_begin_command_buffer();

void renderer_end_command_buffer(VkCommandBuffer *command_buffer);

VkCommandBuffer *renderer_begin_command_buffers(uint32_t *buffer_count);

void renderer_end_command_buffers();

#endif // FLAP_RENDERER_H_
