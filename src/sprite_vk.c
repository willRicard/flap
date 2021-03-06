#include "sprite_impl.h"

#include <vulkan/vulkan.h>

#include <sulfur/buffer.h>
#include <sulfur/device.h>
#include <sulfur/shader.h>
#include <sulfur/swapchain.h>
#include <sulfur/texture.h>

#include "assets_vk.h"
#include "window.h"

static SulfurTexture sprite_texture = {0};
static VkDescriptorSetLayout sprite_descriptor_set_layout = VK_NULL_HANDLE;
static VkPipelineLayout sprite_pipeline_layout = VK_NULL_HANDLE;

static SulfurShader sprite_shaders[2];

static SulfurBuffer sprite_vertex_buffer = {0};
static SulfurBuffer sprite_index_buffer = {0};

void sprite_init(SulfurDevice *dev) {
  assets_vk_create_shader(dev, "shaders/sprite.vert.spv",
                          VK_SHADER_STAGE_VERTEX_BIT, &sprite_shaders[0]);

  assets_vk_create_shader(dev, "shaders/sprite.frag.spv",
                          VK_SHADER_STAGE_FRAGMENT_BIT, &sprite_shaders[1]);

  assets_vk_create_texture(dev, "images/atlas.png", VK_FORMAT_R8G8B8A8_UNORM,
                           &sprite_texture);

  VkDescriptorSetLayoutBinding descriptor_layout_binding = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT};

  VkDescriptorSetLayoutCreateInfo descriptor_layout_info = {0};
  descriptor_layout_info.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptor_layout_info.bindingCount = 1;
  descriptor_layout_info.pBindings = &descriptor_layout_binding;

  VkResult result =
      vkCreateDescriptorSetLayout(dev->device, &descriptor_layout_info, NULL,
                                  &sprite_descriptor_set_layout);
  if (result != VK_SUCCESS) {
    window_fail_with_error("Error initializing sprite descriptor sets: "
                           "vkCreateDescriptorSetLayout");
  }

  VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 1;
  pipeline_layout_info.pSetLayouts = &sprite_descriptor_set_layout;

  result = vkCreatePipelineLayout(dev->device, &pipeline_layout_info, NULL,
                                  &sprite_pipeline_layout);
  if (result != VK_SUCCESS) {
    window_fail_with_error(
        "Error initializing sprite descriptor sets: vkCreatePipelineLayout");
  }

  // Create vertex buffer
  sulfur_buffer_create(dev, sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       &sprite_vertex_buffer);

  sulfur_buffer_create(
      dev, sizeof(indices),
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &sprite_index_buffer);

  SulfurBuffer tmp_buf = {0};
  sulfur_buffer_create(dev, sizeof(indices),
                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       &tmp_buf);

  sulfur_buffer_write(indices, &tmp_buf);

  sulfur_buffer_copy(dev, &tmp_buf, &sprite_index_buffer);
  sulfur_buffer_destroy(dev, &tmp_buf);
}

void sprite_quit(SulfurDevice *dev) {
  vkDeviceWaitIdle(dev->device);

  sulfur_buffer_destroy(dev, &sprite_vertex_buffer);
  sulfur_buffer_destroy(dev, &sprite_index_buffer);

  vkDestroyDescriptorSetLayout(dev->device, sprite_descriptor_set_layout, NULL);

  vkDestroyPipelineLayout(dev->device, sprite_pipeline_layout, NULL);

  sulfur_texture_destroy(dev, &sprite_texture);

  for (uint32_t i = 0; i < 2; i++) {
    sulfur_shader_destroy(dev, &sprite_shaders[i]);
  }
}

void sprite_get_pipeline_create_info(
    VkGraphicsPipelineCreateInfo *pipeline_info) {

  // Add shader stages
  pipeline_info->stageCount = 2;
  pipeline_info->pStages = sprite_shaders;

  pipeline_info->layout = sprite_pipeline_layout;

  // Add vertex attributes
  static VkVertexInputBindingDescription sprite_vertex_binding = {0};
  sprite_vertex_binding.binding = 0;
  sprite_vertex_binding.stride = 4 * sizeof(float);
  sprite_vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  static VkVertexInputAttributeDescription sprite_vertex_attribute = {0};
  sprite_vertex_attribute.location = 0;
  sprite_vertex_attribute.binding = 0;
  sprite_vertex_attribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  sprite_vertex_attribute.offset = 0;

  static VkPipelineVertexInputStateCreateInfo sprite_vertex_input_info = {0};
  sprite_vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  sprite_vertex_input_info.vertexBindingDescriptionCount = 1;
  sprite_vertex_input_info.pVertexBindingDescriptions = &sprite_vertex_binding;
  sprite_vertex_input_info.vertexAttributeDescriptionCount = 1;
  sprite_vertex_input_info.pVertexAttributeDescriptions =
      &sprite_vertex_attribute;

  pipeline_info->pVertexInputState = &sprite_vertex_input_info;
}

VkPipelineLayout sprite_get_pipeline_layout() { return sprite_pipeline_layout; }

VkDescriptorSetLayout sprite_get_descriptor_set_layout(void) {
  return sprite_descriptor_set_layout;
}

void sprite_update() {
  sulfur_buffer_write(vertices, &sprite_vertex_buffer);
}

void sprite_record_command_buffer(VkCommandBuffer cmd_buf) {
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(cmd_buf, 0, 1, &sprite_vertex_buffer.buffer, &offset);
  vkCmdBindIndexBuffer(cmd_buf, sprite_index_buffer.buffer, 0,
                       VK_INDEX_TYPE_UINT16);
  vkCmdDrawIndexed(cmd_buf, kIndicesPerSprite * kNumSprites, 1, 0, 0, 0);
}

void sprite_create_descriptor(SulfurDevice *dev,
                              VkDescriptorSet descriptor_set) {
  VkDescriptorImageInfo image_info = {0};
  image_info.sampler = sprite_texture.sampler;
  image_info.imageView = sprite_texture.image_view;
  image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkWriteDescriptorSet write_info = {0};
  write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write_info.dstSet = descriptor_set;
  write_info.dstBinding = 0;
  write_info.dstArrayElement = 0;
  write_info.descriptorCount = 1;
  write_info.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write_info.pImageInfo = &image_info;

  vkUpdateDescriptorSets(dev->device, 1, &write_info, 0, NULL);
}
