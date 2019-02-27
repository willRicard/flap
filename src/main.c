#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "flap.h"

#include "device.h"
#include "error.h"
#include "game.h"
#include "instance.h"
#include "pipeline.h"
#include "sprite.h"
#include "swapchain.h"
#include "window.h"

// Clear blue sky
const VkClearValue FLAP_CLEAR_COLOR = {{{0.53f, 0.81f, 0.92f, 1.f}}};

static VkInstance instance;
static Device device = {0};
static Swapchain swapchain = {0};

static VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
static VkPipeline pipelines[2] = {0};

static VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

static VkDescriptorSet descriptor_sets[4] = {0};

static void create_pipelines() {
  VkGraphicsPipelineCreateInfo pipeline_infos[2] = {{0}};
  for (uint32_t i = 0; i < 2; i++) {
    pipeline_make_default_create_info(&swapchain, &pipeline_infos[i]);
  }

  sprite_get_pipeline_create_info(&pipeline_infos[0]);

  vkCreateGraphicsPipelines(device.device, pipeline_cache, 1, pipeline_infos,
                            NULL, pipelines);
}

static void create_descriptor_sets() {
  VkDescriptorPoolSize pool_size;
  pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_size.descriptorCount = swapchain.image_count;

  VkDescriptorPoolCreateInfo pool_info = {0};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = 1;
  pool_info.pPoolSizes = &pool_size;
  pool_info.maxSets = swapchain.image_count;

  error_check(
      vkCreateDescriptorPool(device.device, &pool_info, NULL, &descriptor_pool),
      "vkCreateDescriptorPool");

  VkDescriptorSetLayout layouts[3];
  for (uint32_t i = 0; i < swapchain.image_count; ++i) {
    layouts[i] = sprite_get_descriptor_set_layout();
  }

  VkDescriptorSetAllocateInfo alloc_info = {0};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = descriptor_pool;
  alloc_info.descriptorSetCount = swapchain.image_count;
  alloc_info.pSetLayouts = layouts;

  error_check(
      vkAllocateDescriptorSets(device.device, &alloc_info, descriptor_sets),
      "vkAllocateDescriptorSets");

  for (uint32_t i = 0; i < swapchain.image_count; ++i) {
    sprite_create_descriptor(&device, descriptor_sets[i]);
  }
}

static void record_command_buffers() {
  VkCommandBufferBeginInfo begin_info = {0};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  for (uint32_t i = 0; i < swapchain.image_count; i++) {
    VkCommandBuffer cmd_buf = swapchain.command_buffers[i];

    vkBeginCommandBuffer(cmd_buf, &begin_info);

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = swapchain.render_pass;
    render_pass_info.framebuffer = swapchain.framebuffers[i];
    render_pass_info.renderArea.extent = swapchain.info.imageExtent;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &FLAP_CLEAR_COLOR;
    vkCmdBeginRenderPass(cmd_buf, &render_pass_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[0]);

    vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            sprite_get_pipeline_layout(), 0, 1,
                            &descriptor_sets[i], 0, NULL);

    sprite_record_command_buffer(cmd_buf);

    vkCmdEndRenderPass(cmd_buf);

    vkEndCommandBuffer(cmd_buf);
  }
}

int main() {
  srand((unsigned int)time(NULL));

  window_init();

  instance = instance_create();

  VkSurfaceKHR surface = window_create_surface(instance);

  device_create(instance, surface, &device);

  swapchain_create(&device, surface, &swapchain);

  pipeline_cache = pipeline_cache_create(&device);

  sprite_init(&device);

  create_pipelines();

  create_descriptor_sets();

  record_command_buffers();

  game_init();

  while (!window_should_close()) {
    game_update();

    window_update();

    sprite_update();

    if (!swapchain_present(&device, surface, &swapchain)) {
      vkDestroyPipeline(device.device, pipelines[0], NULL);
      vkDestroyPipeline(device.device, pipelines[1], NULL);

      create_pipelines();
      create_descriptor_sets();
      record_command_buffers();
    }
  }

  vkDeviceWaitIdle(device.device);

  vkDestroyDescriptorPool(device.device, descriptor_pool, NULL);

  vkDestroyPipeline(device.device, pipelines[0], NULL);
  vkDestroyPipeline(device.device, pipelines[1], NULL);

  sprite_quit(&device);

  pipeline_cache_destroy(&device, pipeline_cache);

  swapchain_destroy(&device, &swapchain);

  device_destroy(&device);

  window_quit();
  vkDestroySurfaceKHR(instance, surface, NULL);
  vkDestroyInstance(instance, NULL);

  return 0;
}
