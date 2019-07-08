#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sulfur/device.h>
#include <sulfur/instance.h>
#include <sulfur/pipeline.h>
#include <sulfur/swapchain.h>

#include "assets.h"
#include "flap.h"
#include "game.h"
#include "sprite.h"
#include "window.h"

// Clear blue sky
const VkClearValue FLAP_CLEAR_COLOR = {{{0.53f, 0.81f, 0.92f, 1.f}}};

static VkInstance instance;
static SulfurDevice device = {};
static SulfurSwapchain swapchain = {};

static VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
static VkPipeline pipelines[2] = {};

static VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

static VkDescriptorSet descriptor_sets[4] = {};

static void create_pipelines() {
  VkGraphicsPipelineCreateInfo pipeline_infos[2] = {};
  for (uint32_t i = 0; i < 2; i++) {
    sulfur_pipeline_make_default_create_info(&swapchain, &pipeline_infos[i]);
  }

  sprite_get_pipeline_create_info(&pipeline_infos[0]);

  vkCreateGraphicsPipelines(device.device, pipeline_cache, 1, pipeline_infos,
                            NULL, pipelines);
}

static void create_descriptor_sets() {
  VkDescriptorPoolSize pool_size;
  pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_size.descriptorCount = swapchain.image_count;

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = 1;
  pool_info.pPoolSizes = &pool_size;
  pool_info.maxSets = swapchain.image_count;

  VkResult result =
      vkCreateDescriptorPool(device.device, &pool_info, NULL, &descriptor_pool);
  if (result != VK_SUCCESS) {
    window_fail_with_error("vkCreateDescriptorPool");
  }

  VkDescriptorSetLayout layouts[3];
  for (uint32_t i = 0; i < swapchain.image_count; ++i) {
    layouts[i] = sprite_get_descriptor_set_layout();
  }

  VkDescriptorSetAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = descriptor_pool;
  alloc_info.descriptorSetCount = swapchain.image_count;
  alloc_info.pSetLayouts = layouts;

  result =
      vkAllocateDescriptorSets(device.device, &alloc_info, descriptor_sets);
  if (result != VK_SUCCESS) {
    window_fail_with_error("vkAllocateDescriptorSets");
  }

  for (uint32_t i = 0; i < swapchain.image_count; ++i) {
    sprite_create_descriptor(&device, descriptor_sets[i]);
  }
}

static void record_command_buffers() {
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  for (uint32_t i = 0; i < swapchain.image_count; i++) {
    VkCommandBuffer cmd_buf = swapchain.command_buffers[i];

    vkBeginCommandBuffer(cmd_buf, &begin_info);

    VkRenderPassBeginInfo render_pass_info = {};
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

  static const VkApplicationInfo app_info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "Flap",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "Sulfur",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_MAKE_VERSION(1, 0, 0)};

  uint32_t surface_extension_count = 0;
  const char **surface_extensions =
      window_get_extensions(&surface_extension_count);

  if (sulfur_instance_create(&app_info, surface_extension_count,
                             surface_extensions, &instance) != VK_SUCCESS) {
    window_fail_with_error("vkCreateInstance");
    return 1;
  }

  VkSurfaceKHR surface = window_create_surface(instance);

  sulfur_device_create(instance, surface, &device);

  sulfur_swapchain_create(&device, surface, &swapchain);

  assets_create_pipeline_cache(&device, &pipeline_cache);

  sprite_init(&device);

  create_pipelines();

  create_descriptor_sets();

  record_command_buffers();

  game_init();

  while (!window_should_close()) {
    game_update();

    window_update();

    sprite_update();

    if (!sulfur_swapchain_present(&device, surface, &swapchain)) {
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

  assets_destroy_pipeline_cache(&device, pipeline_cache);

  sulfur_swapchain_destroy(&device, &swapchain);

  sulfur_device_destroy(&device);

  window_quit();
  vkDestroySurfaceKHR(instance, surface, NULL);
  vkDestroyInstance(instance, NULL);

  return 0;
}
