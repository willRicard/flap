#ifndef FLAP_DEVICE_H
#define FLAP_DEVICE_H
#include <vulkan/vulkan.h>

typedef struct Device {
  VkDevice device;

  VkPhysicalDevice physical_device;
  VkPhysicalDeviceProperties physical_device_properties;
  VkPhysicalDeviceMemoryProperties physical_device_memory_properties;

  uint32_t graphics_queue_id;
  uint32_t present_queue_id;
  VkQueue graphics_queue;
  VkQueue present_queue;

  VkCommandPool command_pool;
} Device;

/**
 * Create a device for presenting to a window surface.
 * @param VkSurfaceKHR surface The surface to present to.
 */
void device_create(VkInstance instance, VkSurfaceKHR surface, Device *device);

/**
 * Destroy a device.
 */
void device_destroy(Device *device);

/**
 * Find the device memory type most fit for a memory allocation.
 * @param VkMemoryRequirements mem_requirements Size of the allocation.
 * @param VkMemoryPropertyFlags mem_properties Specific memory properties
 * required for the allocated memory.
 */
uint32_t device_find_memory_type(Device *device,
                                 VkMemoryRequirements mem_requirements,
                                 VkMemoryPropertyFlags mem_properties);

/**
 * Begin a one time command buffer.
 */
void device_begin_command_buffer(Device *device, VkCommandBuffer *buffer);

/**
 * Submit and destroy a one time command buffer.
 */
void device_end_command_buffer(Device *device, VkCommandBuffer *buffer);

#endif // FLAP_DEVICE_H
