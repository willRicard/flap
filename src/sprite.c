#include "sprite.h"

#include <vulkan/vulkan.h>

#include <sulfur/buffer.h>
#include <sulfur/device.h>
#include <sulfur/shader.h>
#include <sulfur/swapchain.h>
#include <sulfur/texture.h>

#include "assets.h"
#include "flap.h"
#include "window.h"

static unsigned int sprite_count = 0;

static Sprite sprite_vertices[1 + 4 * FLAP_NUM_PIPES];

static const uint16_t sprite_indices[] = {
    0,   1,   2,   2,   0,   3,   4,   5,   6,   6,   4,   7,   8,   9,   10,
    10,  8,   11,  12,  13,  14,  14,  12,  15,  16,  17,  18,  18,  16,  19,
    20,  21,  22,  22,  20,  23,  24,  25,  26,  26,  24,  27,  28,  29,  30,
    30,  28,  31,  32,  33,  34,  34,  32,  35,  36,  37,  38,  38,  36,  39,
    40,  41,  42,  42,  40,  43,  44,  45,  46,  46,  44,  47,  48,  49,  50,
    50,  48,  51,  52,  53,  54,  54,  52,  55,  56,  57,  58,  58,  56,  59,
    60,  61,  62,  62,  60,  63,  64,  65,  66,  66,  64,  67,  68,  69,  70,
    70,  68,  71,  72,  73,  74,  74,  72,  75,  76,  77,  78,  78,  76,  79,
    80,  81,  82,  82,  80,  83,  84,  85,  86,  86,  84,  87,  88,  89,  90,
    90,  88,  91,  92,  93,  94,  94,  92,  95,  96,  97,  98,  98,  96,  99,
    100, 101, 102, 102, 100, 103, 104, 105, 106, 106, 104, 107, 108, 109, 110,
    110, 108, 111, 112, 113, 114, 114, 112, 115, 116, 117, 118, 118, 116, 119,
    120, 121, 122, 122, 120, 123, 124, 125, 126, 126, 124, 127, 128, 129, 130,
    130, 128, 131, 132, 133, 134, 134, 132, 135, 136, 137, 138, 138, 136, 139,
    140, 141, 142, 142, 140, 143, 144, 145, 146, 146, 144, 147, 148, 149, 150,
    150, 148, 151, 152, 153, 154, 154, 152, 155, 156, 157, 158, 158, 156, 159,
    160, 161, 162, 162, 160, 163, 164, 165, 166, 166, 164, 167,
};

static SulfurTexture sprite_texture = {};
static VkDescriptorSetLayout sprite_descriptor_set_layout = VK_NULL_HANDLE;
static VkPipelineLayout sprite_pipeline_layout = VK_NULL_HANDLE;

static SulfurShader sprite_shaders[2];

static SulfurBuffer sprite_vertex_buffer = {};
static SulfurBuffer sprite_index_buffer = {};

void sprite_init(SulfurDevice *dev) {
  VkResult result =
      assets_create_shader(dev, "shaders/sprite.vert.spv",
                           VK_SHADER_STAGE_VERTEX_BIT, &sprite_shaders[0]);
  if (result != VK_SUCCESS) {
    window_fail_with_error("Error loading sprite vertex shader.");
  }
  result =
      assets_create_shader(dev, "shaders/sprite.frag.spv",
                           VK_SHADER_STAGE_FRAGMENT_BIT, &sprite_shaders[1]);
  if (result != VK_SUCCESS) {
    window_fail_with_error("Error loading sprite fragment shader");
  }

  result = assets_create_texture(dev, "images/atlas.png",
                                 VK_FORMAT_R8G8B8A8_UNORM, &sprite_texture);
  if (result != VK_SUCCESS) {
    window_fail_with_error("Error loading sprite images.");
  }

  VkDescriptorSetLayoutBinding descriptor_layout_binding = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT};

  VkDescriptorSetLayoutCreateInfo descriptor_layout_info = {};
  descriptor_layout_info.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptor_layout_info.bindingCount = 1;
  descriptor_layout_info.pBindings = &descriptor_layout_binding;

  result = vkCreateDescriptorSetLayout(dev->device, &descriptor_layout_info,
                                       NULL, &sprite_descriptor_set_layout);
  if (result != VK_SUCCESS) {
    window_fail_with_error("Error initializing sprite descriptor sets: "
                           "vkCreateDescriptorSetLayout");
  }

  VkPipelineLayoutCreateInfo pipeline_layout_info = {};
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
  sulfur_buffer_create(dev, sizeof(sprite_vertices),
                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       &sprite_vertex_buffer);

  sulfur_buffer_create(
      dev, sizeof(sprite_indices),
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &sprite_index_buffer);

  SulfurBuffer tmp_buf = {};
  sulfur_buffer_create(dev, sizeof(sprite_indices),
                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       &tmp_buf);

  sulfur_buffer_write(sprite_indices, &tmp_buf);

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
  static VkVertexInputBindingDescription sprite_vertex_binding = {};
  sprite_vertex_binding.binding = 0;
  sprite_vertex_binding.stride = 4 * sizeof(float);
  sprite_vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  static VkVertexInputAttributeDescription sprite_vertex_attribute = {};
  sprite_vertex_attribute.location = 0;
  sprite_vertex_attribute.binding = 0;
  sprite_vertex_attribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  sprite_vertex_attribute.offset = 0;

  static VkPipelineVertexInputStateCreateInfo sprite_vertex_input_info = {};
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
  sulfur_buffer_write(sprite_vertices, &sprite_vertex_buffer);
}

void sprite_record_command_buffer(VkCommandBuffer cmd_buf) {
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(cmd_buf, 0, 1, &sprite_vertex_buffer.buffer, &offset);
  vkCmdBindIndexBuffer(cmd_buf, sprite_index_buffer.buffer, 0,
                       VK_INDEX_TYPE_UINT16);
  vkCmdDrawIndexed(cmd_buf, 6 * (1 + 4 * FLAP_NUM_PIPES), 1, 0, 0, 0);
}

void sprite_create_descriptor(SulfurDevice *dev,
                              VkDescriptorSet descriptor_set) {
  VkDescriptorImageInfo image_info = {};
  image_info.sampler = sprite_texture.sampler;
  image_info.imageView = sprite_texture.image_view;
  image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkWriteDescriptorSet write_info = {};
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

void sprite_set_th(Sprite *sprite, float th) {
  float ty = sprite->vertices[0].ty;
  sprite->vertices[1].ty = ty + th;
  sprite->vertices[2].ty = ty + th;
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
