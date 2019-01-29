#include "buffer.h"

#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "flap.h"
#include "window.h"

void buffer_create(Device *dev, VkDeviceSize size, VkBufferUsageFlags usage,
                   VkMemoryPropertyFlags memory_properties, Buffer *buffer) {
  VkPhysicalDeviceProperties props = {0};
  vkGetPhysicalDeviceProperties(dev->physical_device, &props);
  VkDeviceSize alignment = props.limits.nonCoherentAtomSize;
  VkDeviceSize actual_size = (size / alignment + 1) * alignment;
  buffer->size = actual_size;

  VkBufferCreateInfo buffer_info = {0};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = actual_size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  error_check(vkCreateBuffer(dev->device, &buffer_info, NULL, &buffer->buffer),
              "An error occured while creating a buffer.");

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(dev->device, buffer->buffer,
                                &memory_requirements);

  uint32_t best_memory =
      device_find_memory_type(dev, memory_requirements, memory_properties);

  VkMemoryAllocateInfo allocate_info = {0};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = memory_requirements.size;
  allocate_info.memoryTypeIndex = best_memory;

  vkAllocateMemory(dev->device, &allocate_info, NULL, &buffer->memory);
  vkBindBufferMemory(dev->device, buffer->buffer, buffer->memory, 0);

  if (memory_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
    error_check(vkMapMemory(dev->device, buffer->memory, 0, actual_size, 0,
                            (void **)&buffer->data),
                "vkMapMemory");
  }
}

void buffer_destroy(Device *dev, Buffer *buffer) {
  if (buffer->data != NULL) {
    vkUnmapMemory(dev->device, buffer->memory);
  }
  vkDestroyBuffer(dev->device, buffer->buffer, NULL);
  vkFreeMemory(dev->device, buffer->memory, NULL);
}

void buffer_write(const void *data, Buffer *buffer) {
  memcpy(buffer->data, data, (size_t)buffer->size);
}

void buffer_copy(Device *dev, Buffer *src_buf, Buffer *dst_buf) {
  VkCommandBuffer cmd_buf = VK_NULL_HANDLE;
  device_begin_command_buffer(dev, &cmd_buf);

  VkBufferCopy copy_region = {0};
  copy_region.srcOffset = 0;
  copy_region.dstOffset = 0;
  copy_region.size = src_buf->size;
  vkCmdCopyBuffer(cmd_buf, src_buf->buffer, dst_buf->buffer, 1, &copy_region);

  device_end_command_buffer(dev, &cmd_buf);
}
