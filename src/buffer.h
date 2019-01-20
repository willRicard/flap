#ifndef FLAP_BUFFER_H_
#define FLAP_BUFFER_H_
#include <vulkan/vulkan.h>

/**
 * Buffer type.
 * Tells where to store the buffer data.
 */
typedef enum {
  BUFFER_TYPE_HOST_BUFFER = 0, /** Only use host memory */
  BUFFER_TYPE_STAGED_BUFFER,   /** Use both host and GPU memory */
  BUFFER_TYPE_DEVICE_BUFFER    /** Only use GPU memory */
} BufferType;

/**
 * Buffer interface for common buffer workflows
 */
typedef struct Buffer {
  BufferType type;
  VkBufferUsageFlags usage;

  VkBuffer buffer;
  VkDeviceMemory memory;
  uint8_t *data; /** Host memory */

  VkBuffer staging_buffer; /** Host buffer for staged Buffers */
  VkDeviceMemory staging_memory;
} Buffer;

/**
 * Create a buffer and allocate memory on the device.
 */
void buffer_create(Buffer *buf, VkDeviceSize size, VkBufferUsageFlags usage,
                   BufferType type);

/**
 * Free resources associated with a buffer
 */
void buffer_destroy(Buffer *buf);

/**
 * Write data to the buffer memory
 */
void buffer_write(Buffer buf, VkDeviceSize size, const void *data);

#endif // FLAP_BUFFER_H_
