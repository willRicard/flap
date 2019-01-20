#include "buffer.h"

#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "error.h"
#include "flap.h"
#include "renderer.h"
#include "window.h"

// Create a VkBuffer, then allocate and bind VkDeviceMemory matching the buffer
// memory requirements.
static void make_vk_buffer(VkBuffer *buffer, VkDeviceMemory *memory,
                           VkDeviceSize size, VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags memory_properties) {
  VkDevice device = renderer_get_device();
  VkPhysicalDeviceMemoryProperties physical_device_memory_properties =
      renderer_get_physical_device_memory_properties();

  VkBufferCreateInfo buffer_info = {0};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VK_CHECK(vkCreateBuffer(device, &buffer_info, NULL, buffer),
           "An error occured while creating a buffer.");

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(device, *buffer, &memory_requirements);

  uint32_t best_memory = 0;
  VkBool32 memory_found = VK_FALSE;
  for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount;
       i++) {
    VkMemoryType memory_type = physical_device_memory_properties.memoryTypes[i];
    if ((memory_requirements.memoryTypeBits & (1 << i)) &&
        (memory_type.propertyFlags & memory_properties)) {
      best_memory = i;
      memory_found = VK_TRUE;
      break;
    }
  }
  if (memory_found == VK_FALSE) {
    fail_with_error(
        "Allocation failed: no suitable memory type could be found!");
  }

  VkMemoryAllocateInfo allocate_info = {0};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = memory_requirements.size;
  allocate_info.memoryTypeIndex = best_memory;

  vkAllocateMemory(device, &allocate_info, NULL, memory);
  vkBindBufferMemory(device, *buffer, *memory, 0);
}

// Destroy a VkBuffer and free the bound VkDeviceMemory.
static void free_vk_buffer(VkBuffer buffer, VkDeviceMemory memory) {
  VkDevice device = renderer_get_device();
  vkDestroyBuffer(device, buffer, NULL);
  vkFreeMemory(device, memory, NULL);
}

// Synchronously copy data from one VkBuffer to another.
static void copy_vk_buffer(VkBuffer src_buf, VkBuffer dst_buf,
                           VkDeviceSize size) {
  VkCommandBuffer *cmd_buf = renderer_begin_command_buffer();

  VkBufferCopy copy_region = {0};
  copy_region.srcOffset = 0;
  copy_region.dstOffset = 0;
  copy_region.size = size;
  vkCmdCopyBuffer(*cmd_buf, src_buf, dst_buf, 1, &copy_region);

  renderer_end_command_buffer(cmd_buf);
}

void host_buffer_create(Buffer *buffer, VkDeviceSize size,
                        VkBufferUsageFlags usage) {
  make_vk_buffer(&buffer->buffer, &buffer->memory, size, usage,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  VK_CHECK(vkMapMemory(renderer_get_device(), buffer->memory, 0, size, 0,
                       (void **)&buffer->data),
           "An error occured while mapping device memory.")
}

void host_buffer_write(Buffer buffer, VkDeviceSize size, const void *data) {
  memcpy(buffer.data, data, (size_t)size);
}

void host_buffer_destroy(Buffer *buffer) {
  vkUnmapMemory(renderer_get_device(), buffer->memory);
  free_vk_buffer(buffer->buffer, buffer->memory);
}

void staged_buffer_create(Buffer *buffer, VkDeviceSize size,
                          VkBufferUsageFlags usage) {

  make_vk_buffer(&buffer->buffer, &buffer->memory, size,
                 usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  make_vk_buffer(&buffer->staging_buffer, &buffer->staging_memory, size,
                 usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,

                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  VK_CHECK(vkMapMemory(renderer_get_device(), buffer->staging_memory, 0, size,
                       0, (void **)&buffer->data),
           "An error occured while mapping device memory.")
}

void staged_buffer_write(Buffer buffer, VkDeviceSize size, const void *data) {
  memcpy(buffer.data, data, (size_t)size);
  copy_vk_buffer(buffer.staging_buffer, buffer.buffer, size);
}

void staged_buffer_destroy(Buffer *buffer) {
  vkUnmapMemory(renderer_get_device(), buffer->staging_memory);

  free_vk_buffer(buffer->buffer, buffer->memory);
  free_vk_buffer(buffer->staging_buffer, buffer->staging_memory);
}

void device_buffer_create(Buffer *buffer, VkDeviceSize size,
                          VkBufferUsageFlags usage) {
  make_vk_buffer(&buffer->buffer, &buffer->memory, size,
                 usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void device_buffer_write(Buffer buffer, VkDeviceSize size, const void *data) {
  VkDevice device = renderer_get_device();

  VkBuffer tmp_buf = VK_NULL_HANDLE;
  VkDeviceMemory tmp_mem = VK_NULL_HANDLE;
  void *tmp_data = NULL;

  make_vk_buffer(&tmp_buf, &tmp_mem, size,
                 buffer.usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  vkMapMemory(renderer_get_device(), tmp_mem, 0, size, 0, &tmp_data);

  memcpy(tmp_data, data, (size_t)size);

  copy_vk_buffer(tmp_buf, buffer.buffer, size);

  vkUnmapMemory(device, tmp_mem);
  free_vk_buffer(tmp_buf, tmp_mem);
}

void device_buffer_destroy(Buffer *buffer) {
  free_vk_buffer(buffer->buffer, buffer->memory);
}

typedef void (*BufferCreateFunction)(Buffer *buffer, VkDeviceSize size,
                                     VkBufferUsageFlags usage);
typedef void (*BufferDestroyFunction)(Buffer *buffer);
typedef void (*BufferWriteFunction)(Buffer buffer, VkDeviceSize size,
                                    const void *data);

static const BufferCreateFunction buffer_create_functions[] = {
    host_buffer_create, staged_buffer_create, device_buffer_create};

static const BufferDestroyFunction buffer_destroy_functions[] = {
    host_buffer_destroy, staged_buffer_destroy, device_buffer_destroy};

static const BufferWriteFunction buffer_write_functions[] = {
    host_buffer_write, staged_buffer_write, device_buffer_write};

void buffer_create(Buffer *buffer, VkDeviceSize size, VkBufferUsageFlags usage,
                   BufferType type) {
  buffer->type = type;
  buffer->usage = usage;

  buffer_create_functions[type](buffer, size, usage);
}

void buffer_destroy(Buffer *buffer) {
  buffer_destroy_functions[buffer->type](buffer);
}

void buffer_write(Buffer buffer, VkDeviceSize size, const void *data) {
  buffer_write_functions[buffer.type](buffer, size, data);
}
