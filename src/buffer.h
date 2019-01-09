#ifndef FLAP_BUFFER_H_
#define FLAP_BUFFER_H_
#include <vulkan/vulkan.h>

typedef enum {
  BUFFER_TYPE_HOST_BUFFER = 0,
  BUFFER_TYPE_STAGED_BUFFER,
  BUFFER_TYPE_DEVICE_BUFFER
} BufferType;

typedef struct Buffer {
  BufferType type;
  VkBufferUsageFlags usage;

  VkBuffer buffer;
  VkDeviceMemory memory;
  uint8_t *data;

  VkBuffer staging_buffer;
  VkDeviceMemory staging_memory;

  VkBuffer gpu_buffer;
  VkDeviceMemory gpu_memory;
} Buffer;

void buffer_create(Buffer *buf, VkDeviceSize size, VkBufferUsageFlags usage,
                   BufferType type);

void buffer_destroy(Buffer *buf);

void buffer_write(Buffer buf, VkDeviceSize size, const void *data);

#endif // FLAP_BUFFER_H_
