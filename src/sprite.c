#include "sprite.h"

#include <vulkan/vulkan.h>

#include "buffer.h"
#include "device.h"
#include "error.h"
#include "flap.h"
#include "shader.h"
#include "swapchain.h"
#include "texture.h"

static unsigned int sprite_count = 0;

static Sprite sprite_vertices[1 + FLAP_NUM_PIPES * 2] = {{{{0}}}};

static const uint16_t sprite_indices[] = {
    0,  1,  2,  2,  0,  3,  4,  5,  6,  6,  4,  7,  8,  9,  10, 10, 8,  11,
    12, 13, 14, 14, 12, 15, 16, 17, 18, 18, 16, 19, 20, 21, 22, 22, 20, 23,
    24, 25, 26, 26, 24, 27, 28, 29, 30, 30, 28, 31, 32, 33, 34, 34, 32, 35,
    36, 37, 38, 38, 36, 39, 40, 41, 42, 42, 40, 43, 44, 45, 46, 46, 44, 47,
    48, 49, 50, 50, 48, 51, 52, 53, 54, 54, 52, 55, 56, 57, 58, 58, 56, 59,
    60, 61, 62, 62, 60, 63, 64, 65, 66, 66, 64, 67, 68, 69, 70, 70, 68, 71,
    72, 73, 74, 74, 72, 75, 76, 77, 78, 78, 76, 79, 80, 81, 82, 82, 80, 83,
};

static Texture sprite_texture = {0};
static VkDescriptorSetLayout sprite_descriptor_set_layout = VK_NULL_HANDLE;
static VkPipelineLayout sprite_pipeline_layout = VK_NULL_HANDLE;

static Shader sprite_shaders[2] = {{0}};

static Buffer sprite_vertex_buffer = {0};
static Buffer sprite_index_buffer = {0};

void sprite_init(Device *dev) {
  // Load shaders
  shader_create(dev, "shaders/sprite.vert.spv", VK_SHADER_STAGE_VERTEX_BIT,
                &sprite_shaders[0]);
  shader_create(dev, "shaders/sprite.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT,
                &sprite_shaders[1]);

  texture_create_from_image(dev, "images/atlas.png", VK_FORMAT_R8G8B8A8_UNORM,
                            &sprite_texture);

  VkDescriptorSetLayoutBinding descriptor_layout_binding = {0};
  descriptor_layout_binding.binding = 0;
  descriptor_layout_binding.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptor_layout_binding.descriptorCount = 1;
  descriptor_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo descriptor_layout_info = {0};
  descriptor_layout_info.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptor_layout_info.bindingCount = 1;
  descriptor_layout_info.pBindings = &descriptor_layout_binding;

  error_check(vkCreateDescriptorSetLayout(dev->device, &descriptor_layout_info,
                                          NULL, &sprite_descriptor_set_layout),
              "vkCreateDescriptorSetLayout");

  VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 1;
  pipeline_layout_info.pSetLayouts = &sprite_descriptor_set_layout;

  error_check(vkCreatePipelineLayout(dev->device, &pipeline_layout_info, NULL,
                                     &sprite_pipeline_layout),
              "vkCreatePipelineLayout");

  // Create vertex buffer
  buffer_create(dev, sizeof(sprite_vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &sprite_vertex_buffer);

  buffer_create(dev, sizeof(sprite_indices),
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &sprite_index_buffer);

  Buffer tmp_buf = {0};
  buffer_create(dev, sizeof(sprite_indices),
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &tmp_buf);

  buffer_write(sprite_indices, &tmp_buf);

  buffer_copy(dev, &tmp_buf, &sprite_index_buffer);
  buffer_destroy(dev, &tmp_buf);
}

void sprite_quit(Device *dev) {
  vkDeviceWaitIdle(dev->device);

  buffer_destroy(dev, &sprite_vertex_buffer);
  buffer_destroy(dev, &sprite_index_buffer);

  vkDestroyDescriptorSetLayout(dev->device, sprite_descriptor_set_layout, NULL);

  vkDestroyPipelineLayout(dev->device, sprite_pipeline_layout, NULL);

  texture_destroy(dev, &sprite_texture);

  for (uint32_t i = 0; i < 2; i++) {
    shader_destroy(dev, &sprite_shaders[i]);
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

  static VkVertexInputAttributeDescription sprite_vertex_attributes[2] = {{0}};
  sprite_vertex_attributes[0].location = 0;
  sprite_vertex_attributes[0].binding = 0;
  sprite_vertex_attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
  sprite_vertex_attributes[0].offset = 0;

  sprite_vertex_attributes[1].location = 1;
  sprite_vertex_attributes[1].binding = 0;
  sprite_vertex_attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
  sprite_vertex_attributes[1].offset = 2 * sizeof(float);

  static VkPipelineVertexInputStateCreateInfo sprite_vertex_input_info = {0};
  sprite_vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  sprite_vertex_input_info.vertexBindingDescriptionCount = 1;
  sprite_vertex_input_info.pVertexBindingDescriptions = &sprite_vertex_binding;
  sprite_vertex_input_info.vertexAttributeDescriptionCount = 2;
  sprite_vertex_input_info.pVertexAttributeDescriptions =
      sprite_vertex_attributes;

  pipeline_info->pVertexInputState = &sprite_vertex_input_info;
}

VkPipelineLayout sprite_get_pipeline_layout() { return sprite_pipeline_layout; }

VkDescriptorSetLayout sprite_get_descriptor_set_layout(void) {
  return sprite_descriptor_set_layout;
}

void sprite_update() { buffer_write(sprite_vertices, &sprite_vertex_buffer); }

void sprite_record_command_buffer(VkCommandBuffer cmd_buf) {
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(cmd_buf, 0, 1, &sprite_vertex_buffer.buffer, &offset);
  vkCmdBindIndexBuffer(cmd_buf, sprite_index_buffer.buffer, 0,
                       VK_INDEX_TYPE_UINT16);
  vkCmdDrawIndexed(cmd_buf, 6 * (1 + FLAP_NUM_PIPES * 2), 1, 0, 0, 0);
}

void sprite_create_descriptor(Device *dev, VkDescriptorSet descriptor_set) {
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

Sprite *sprite_new(int x, int y, int w, int h) {
  Sprite *sprite = &sprite_vertices[sprite_count++];

  sprite->vertices[0].tx = (float)x / sprite_texture.width;
  sprite->vertices[0].ty = (float)y / sprite_texture.height;

  sprite->vertices[1].tx = (float)x / (float)sprite_texture.width;
  sprite->vertices[1].ty = (float)(y + h) / (float)sprite_texture.height;

  sprite->vertices[2].tx = (float)(x + w) / (float)sprite_texture.width;
  sprite->vertices[2].ty = (float)(y + h) / (float)sprite_texture.height;

  sprite->vertices[3].tx = (float)(x + w) / (float)sprite_texture.width;
  sprite->vertices[3].ty = (float)y / (float)sprite_texture.height;

  return sprite;
}

void sprite_set_x(Sprite *sprite, float x) {
  float w = sprite->vertices[2].x - sprite->vertices[0].x;
  sprite->vertices[0].x = x;
  sprite->vertices[1].x = x;
  sprite->vertices[2].x = x + w;
  sprite->vertices[3].x = x + w;
}

void sprite_set_y(Sprite *sprite, float y) {
  float h = sprite->vertices[1].y - sprite->vertices[0].y;
  sprite->vertices[0].y = y;
  sprite->vertices[1].y = y + h;
  sprite->vertices[2].y = y + h;
  sprite->vertices[3].y = y;
}

void sprite_set_w(Sprite *sprite, float w) {
  float x = sprite->vertices[0].x;
  sprite->vertices[2].x = x + w;
  sprite->vertices[3].x = x + w;
}

void sprite_set_h(Sprite *sprite, float h) {
  float y = sprite->vertices[0].y;
  sprite->vertices[1].y = y + h;
  sprite->vertices[2].y = y + h;
}

float sprite_get_x(Sprite *sprite) { return sprite->vertices[0].x; }

float sprite_get_y(Sprite *sprite) { return sprite->vertices[0].y; }

float sprite_get_w(Sprite *sprite) {
  return sprite->vertices[2].x - sprite->vertices[1].x;
}

float sprite_get_h(Sprite *sprite) {
  return sprite->vertices[1].y - sprite->vertices[0].y;
}

int sprite_intersect(Sprite *s1, Sprite *s2) {
  float x1 = sprite_get_x(s1);
  float y1 = sprite_get_y(s1);
  float w1 = sprite_get_w(s1);
  float h1 = sprite_get_h(s1);
  float x2 = sprite_get_x(s2);
  float y2 = sprite_get_y(s2);
  float w2 = sprite_get_w(s2);
  float h2 = sprite_get_h(s2);
  return (x1 < x2 + w2 && x2 < x1 + w1 && y1 < y2 + h2 && y2 < y1 + h1);
}
