#ifndef FLAP_BUFFER_H_
#define FLAP_BUFFER_H_
#include <vulkan/vulkan.h>

#include "device.h"

/**
 * GPU or host buffer
 */
typedef struct Buffer {
  VkBuffer buffer;
  VkDeviceMemory memory;
  VkDeviceSize size;
  uint8_t *data;
} Buffer;

/**
 * Create a buffer and allocate memory on the device.
 */
void buffer_create(Device *device, VkDeviceSize size, VkBufferUsageFlags usage,
                   VkMemoryPropertyFlags memory_properties, Buffer *buf);

/**
 * Destroy a buffer and free the bound memory.
 */
void buffer_destroy(Device *device, Buffer *buf);

/**
 * Write data to a buffer.
 */
void buffer_write(const void *data, Buffer *buf);

/**
 * Synchronous copy from one buffer to another.
 */
void buffer_copy(Device *device, Buffer *src_buf, Buffer *dst_buf);

#endif // FLAP_BUFFER_H_
