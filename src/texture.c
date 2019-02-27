#include "texture.h"

#include "assets.h"
#include "buffer.h"
#include "error.h"
#include "window.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void texture_create(Device *device, int width, int height, VkFormat format,
                    Texture *texture) {
  texture->width = width;
  texture->height = height;

  VkImageCreateInfo image_info = {0};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.format = format;
  image_info.extent.width = width;
  image_info.extent.height = height;
  image_info.extent.depth = 1;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  error_check(vkCreateImage(device->device, &image_info, NULL, &texture->image),
              "vkCreateImage");

  VkMemoryRequirements mem_requirements = {0};
  vkGetImageMemoryRequirements(device->device, texture->image,
                               &mem_requirements);

  uint32_t best_memory = device_find_memory_type(
      device, mem_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VkMemoryAllocateInfo alloc_info = {0};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = best_memory;

  error_check(vkAllocateMemory(device->device, &alloc_info, NULL,
                               &texture->image_memory),
              "vkAllocateMemory");

  vkBindImageMemory(device->device, texture->image, texture->image_memory, 0);

  // Create an image view to access the texture.
  VkImageViewCreateInfo view_info = {0};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = texture->image;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = format;
  view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;

  error_check(
      vkCreateImageView(device->device, &view_info, NULL, &texture->image_view),
      "vkCreateImageView");

  // Create a sampler.
  VkSamplerCreateInfo sampler_info = {0};
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.magFilter = VK_FILTER_NEAREST;
  sampler_info.minFilter = VK_FILTER_NEAREST;
  sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
  sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.mipLodBias = 0;
  sampler_info.anisotropyEnable = VK_FALSE;
  sampler_info.maxAnisotropy = 1.0f;
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler_info.minLod = 0.0f;
  sampler_info.maxLod = 0.0f;
  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;

  error_check(
      vkCreateSampler(device->device, &sampler_info, NULL, &texture->sampler),
      "vkCreateSampler");
}

void texture_create_from_image(Device *device, const char *filename,
                               VkFormat format, Texture *texture) {
  size_t size;
  char *data = assets_read_file(filename, &size);

  int width, height;
  stbi_uc *pixels = NULL;
  pixels = stbi_load_from_memory((const stbi_uc *)data, size, &width, &height,
                                 NULL, STBI_rgb_alpha);

  free(data);

  if (pixels == NULL) {
    window_fail_with_error(filename);
  }

  texture_create(device, width, height, format, texture);

  VkDeviceSize image_size = width * height * 4;

  Buffer buffer = {0};
  buffer_create(device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &buffer);

  buffer_write(pixels, &buffer);

  stbi_image_free(pixels);

  texture_transition_layout(device, texture, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // Copy image data
  VkCommandBuffer cmd_buf = VK_NULL_HANDLE;
  device_begin_command_buffer(device, &cmd_buf);

  VkBufferImageCopy region = {0};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset.x = 0;
  region.imageOffset.y = 0;
  region.imageOffset.z = 0;
  region.imageExtent.width = width;
  region.imageExtent.height = height;
  region.imageExtent.depth = 1;

  vkCmdCopyBufferToImage(cmd_buf, buffer.buffer, texture->image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  device_end_command_buffer(device, &cmd_buf);

  texture_transition_layout(device, texture,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  buffer_destroy(device, &buffer);
}

void texture_destroy(Device *device, Texture *texture) {
  vkDestroySampler(device->device, texture->sampler, NULL);
  vkDestroyImageView(device->device, texture->image_view, NULL);
  vkDestroyImage(device->device, texture->image, NULL);
  vkFreeMemory(device->device, texture->image_memory, NULL);
}

void texture_transition_layout(Device *device, Texture *texture,
                               VkImageLayout old_layout,
                               VkImageLayout new_layout) {
  VkCommandBuffer cmd_buf = VK_NULL_HANDLE;
  device_begin_command_buffer(device, &cmd_buf);

  VkImageMemoryBarrier barrier = {0};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.image = texture->image;

  VkPipelineStageFlags source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkPipelineStageFlags destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }

  vkCmdPipelineBarrier(cmd_buf, source_stage, destination_stage, 0, 0, NULL, 0,
                       NULL, 1, &barrier);

  device_end_command_buffer(device, &cmd_buf);
}
