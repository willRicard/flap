#include <sulfur/debug.h>
#include <sulfur/device.h>
#include <sulfur/pipeline.h>
#include <sulfur/swapchain.h>

#include <string.h>

#include "assets_vk.h"
#include "game.h"
#include "sprite_vk.h"
#include "window_vk.h"

// Clear blue sky
static const VkClearValue kFlapClearColor = {{{0.53F, 0.81F, 0.92F, 1.F}}};

static VkInstance instance = VK_NULL_HANDLE;
static SulfurDevice device = {0};
static SulfurSwapchain swapchain = {0};

static VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
static VkPipeline pipelines[2] = {0};

static VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

static VkDescriptorSet descriptor_sets[4] = {0};

static VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;

static VkDebugReportCallbackEXT debug_report_callback = VK_NULL_HANDLE;

static void create_pipelines() {
  VkGraphicsPipelineCreateInfo pipeline_infos[2] = {0};
  for (uint32_t i = 0; i < 2; i++) {
    sulfur_pipeline_make_default_create_info(&swapchain, &pipeline_infos[i]);
  }

  sprite_get_pipeline_create_info(&pipeline_infos[0]);

  vkCreateGraphicsPipelines(device.device, pipeline_cache, 1, pipeline_infos,
                            NULL, pipelines);
}

static void create_descriptor_sets() {
  VkDescriptorPoolSize pool_size = {0};
  pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_size.descriptorCount = swapchain.image_count;

  VkDescriptorPoolCreateInfo pool_info = {0};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = 1;
  pool_info.pPoolSizes = &pool_size;
  pool_info.maxSets = swapchain.image_count;

  VkResult result =
      vkCreateDescriptorPool(device.device, &pool_info, NULL, &descriptor_pool);
  if (result != VK_SUCCESS) {
    window_fail_with_error("vkCreateDescriptorPool");
  }

  VkDescriptorSetLayout layouts[4] = {0};
  for (uint32_t i = 0; i < swapchain.image_count; ++i) {
    layouts[i] = sprite_get_descriptor_set_layout();
  }

  VkDescriptorSetAllocateInfo alloc_info = {0};
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
  static const VkCommandBufferBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT};

  VkRenderPassBeginInfo render_pass_info = {0};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = swapchain.render_pass;
  render_pass_info.renderArea.extent = swapchain.info.imageExtent;
  render_pass_info.clearValueCount = 1;
  render_pass_info.pClearValues = &kFlapClearColor;

  for (uint32_t i = 0; i < swapchain.image_count; i++) {
    VkCommandBuffer cmd_buf = swapchain.command_buffers[i];

    vkBeginCommandBuffer(cmd_buf, &begin_info);

    render_pass_info.framebuffer = swapchain.framebuffers[i];
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

int main(void) {
  window_init();

  static const VkApplicationInfo app_info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "Flap",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "Sulfur",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_MAKE_VERSION(1, 0, 0)};

  uint32_t layer_count = 0;
  const char *layers[3] = {NULL};

#ifndef NDEBUG
  sulfur_debug_get_validation_layers(&layer_count, layers);
#endif

  uint32_t extension_count = 0;
  const char *extensions[3] = {NULL};

  const char **window_extensions = window_vk_get_extensions(&extension_count);
  memcpy(extensions, window_extensions, extension_count * sizeof(const char *));

#ifndef NDEBUG
  VkBool32 debug_utils_available = VK_FALSE;
  uint32_t debug_extension_count = 0;
  sulfur_debug_get_extensions(&debug_extension_count,
                              &extensions[extension_count],
                              &debug_utils_available);
  extension_count += debug_extension_count;
#endif

  VkInstanceCreateInfo instance_info = {0};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pNext = NULL;
  instance_info.flags = 0;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledLayerCount = layer_count;
  instance_info.ppEnabledLayerNames = layers;
  instance_info.enabledExtensionCount = extension_count;
  instance_info.ppEnabledExtensionNames = extensions;

  vkCreateInstance(&instance_info, NULL, &instance);

#ifndef NDEBUG
  if (debug_utils_available) {
    sulfur_debug_messenger_create(instance, window_vk_debug_messenger_callback,
                                  &debug_messenger);
  } else {
    sulfur_debug_report_callback_create(
        instance, window_vk_debug_report_callback, &debug_report_callback);
  }
#endif

  const VkSurfaceKHR surface = window_vk_create_surface(instance);

  sulfur_device_create(instance, surface, &device);

  sulfur_swapchain_create(&device, surface, &swapchain);

  assets_vk_create_pipeline_cache(&device, "pipeline_cache.bin",
                                  &pipeline_cache);

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

  assets_vk_destroy_pipeline_cache(&device, pipeline_cache,
                                   "pipeline_cache.bin");

  sulfur_swapchain_destroy(&device, &swapchain);

  sulfur_device_destroy(&device);

  window_quit();
  vkDestroySurfaceKHR(instance, surface, NULL);

#ifndef NDEBUG
  if (debug_utils_available) {
    sulfur_debug_messenger_destroy(instance, debug_messenger);
  } else {
    sulfur_debug_report_callback_destroy(instance, debug_report_callback);
  }
#endif

  vkDestroyInstance(instance, NULL);

  return 0;
}
