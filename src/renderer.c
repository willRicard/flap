#include "renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flap.h"
#include "pipeline.h"
#include "window.h"

// Vulkan context
static VkInstance instance = VK_NULL_HANDLE;

static VkDevice device = VK_NULL_HANDLE;
static VkPhysicalDevice physical_device = VK_NULL_HANDLE;
static VkPhysicalDeviceMemoryProperties physical_device_memory_properties;

static VkSurfaceKHR surface = VK_NULL_HANDLE;

static uint32_t graphics_queue_id = 0;
static uint32_t present_queue_id = 0;

static VkQueue graphics_queue, present_queue;

// Swapchain data
static VkSwapchainKHR swapchain = VK_NULL_HANDLE;
static VkSurfaceTransformFlagsKHR swapchain_pre_transform =
    VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
static VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
static VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;
static VkColorSpaceKHR image_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
static VkExtent2D image_extent = {0};
static uint32_t image_count = 3, frame_id = 0;
static VkImageView *swapchain_image_views = NULL;

// Render data
static VkRenderPass render_pass = VK_NULL_HANDLE;

static VkFramebuffer *framebuffers = NULL;

static VkCommandPool command_pool = VK_NULL_HANDLE;
static VkCommandBuffer *command_buffers = NULL;

static VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
static VkSemaphore render_finished_semaphore = VK_NULL_HANDLE;

static VkBuffer vertex_buffer = VK_NULL_HANDLE;

static Pipeline *pipeline = NULL;

void renderer_init() {
  // Create the instance
  uint32_t surface_extension_count = 0;
  const char **surface_extensions =
      window_get_extensions(&surface_extension_count);

  VkInstance instance;

  VkApplicationInfo app_info;
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pNext = NULL;
  app_info.pApplicationName = "Flap";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo instance_info;
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pNext = NULL;
  instance_info.flags = 0;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledLayerCount = 0;
  instance_info.ppEnabledLayerNames = NULL;
  instance_info.enabledExtensionCount = surface_extension_count;
  instance_info.ppEnabledExtensionNames = surface_extensions;

  VkResult result = vkCreateInstance(&instance_info, NULL, &instance);
  if (result != VK_SUCCESS) {
    window_fail_with_error(
        "An error occured while creating the Vulkan instance.");
  }

  uint32_t device_count = 0;
  if (vkEnumeratePhysicalDevices(instance, &device_count, NULL) != VK_SUCCESS) {
    window_fail_with_error("An error occured while enumerating devices.");
  }

  VkPhysicalDevice *devices =
      (VkPhysicalDevice *)malloc(device_count * sizeof(VkPhysicalDevice));

  if (vkEnumeratePhysicalDevices(instance, &device_count, devices) !=
      VK_SUCCESS) {
    window_fail_with_error("An error occured while enumerating devices.");
  }

  // Create the device.
  for (uint32_t i = 0; i < device_count; i++) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(devices[i], &properties);
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      physical_device = devices[i];
      break;
    } else if (properties.deviceType ==
               VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
      physical_device = devices[i];
    }
  }
  if (physical_device == VK_NULL_HANDLE) {
    window_fail_with_error("No suitable device was found.");
  }

  free(devices);

  // Query memory properties for later allocations
  vkGetPhysicalDeviceMemoryProperties(physical_device,
                                      &physical_device_memory_properties);

  // Create the window surface
  if (window_create_surface(instance, &surface) != VK_SUCCESS) {
    window_fail_with_error(
        "An error occured while creating the window surface.");
  }

  // Pick the best graphics & present queues
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount,
                                           NULL);

  VkQueueFamilyProperties *queueFamilyProperties =
      (VkQueueFamilyProperties *)malloc(queueFamilyCount *
                                        sizeof(VkQueueFamilyProperties));

  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount,
                                           queueFamilyProperties);

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    VkQueueFamilyProperties queueFamily = queueFamilyProperties[i];
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphics_queue_id = i;
      break;
    } else if (i == queueFamilyCount - 1) {
      window_fail_with_error("No graphics queue was found.");
    }
  }
  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    VkBool32 presentSupported;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                                         &presentSupported);
    if (presentSupported) {
      present_queue_id = i;
      break;
    } else if (i == queueFamilyCount - 1) {
      window_fail_with_error("No present queue was found.");
    }
  }

  free(queueFamilyProperties);

  float priority = 1.f;

  VkDeviceQueueCreateInfo queue_infos[2];

  queue_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_infos[0].pNext = NULL;
  queue_infos[0].flags = 0;
  queue_infos[0].queueFamilyIndex = graphics_queue_id;
  queue_infos[0].queueCount = 1;
  queue_infos[0].pQueuePriorities = &priority;

  queue_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_infos[1].pNext = NULL;
  queue_infos[1].flags = 0;
  queue_infos[1].queueFamilyIndex = present_queue_id;
  queue_infos[1].queueCount = 1;
  queue_infos[1].pQueuePriorities = &priority;

  VkPhysicalDeviceFeatures features = {0};
  vkGetPhysicalDeviceFeatures(physical_device, &features);

  static const char *deviceExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  VkDeviceCreateInfo device_info = {0};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = NULL;
  device_info.flags = 0;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = queue_infos;
  device_info.enabledLayerCount = 0;
  device_info.ppEnabledLayerNames = NULL;
  device_info.enabledExtensionCount = 1;
  device_info.ppEnabledExtensionNames = &deviceExtension;
  device_info.pEnabledFeatures = &features;

  vkCreateDevice(physical_device, &device_info, NULL, &device);

  vkGetDeviceQueue(device, graphics_queue_id, 0, &graphics_queue);
  vkGetDeviceQueue(device, present_queue_id, 0, &present_queue);

  pipeline_cache_init();

  // Pick the best image format
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount,
                                       NULL);

  VkSurfaceFormatKHR *formats =
      (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));

  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount,
                                       formats);

  if (formatCount != 1 || formats[0].format != VK_FORMAT_UNDEFINED) {
    image_format = formats[0].format;
    image_color_space = formats[0].colorSpace;
  }

  free(formats);

  // Choose the best present mode (see the spec, preference in descending
  // order).
  // 1. Mailbox
  // 2. Immediate
  // 3. FIFO
  uint32_t presentModeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                            &presentModeCount, NULL);

  VkPresentModeKHR *presentModes =
      (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));

  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                            &presentModeCount, presentModes);

  for (uint32_t i = 0; i < presentModeCount; i++) {
    if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
      break;
    } else if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
  }

  free(presentModes);

  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
                                            &capabilities);

  // Choose the best image count
  if (image_count < capabilities.minImageCount) {
    image_count = capabilities.minImageCount;
  } else if (image_count > capabilities.maxImageCount) {
    image_count = capabilities.maxImageCount;
  }

  framebuffers = (VkFramebuffer *)malloc(image_count * sizeof(VkFramebuffer));

  command_buffers =
      (VkCommandBuffer *)malloc(image_count * sizeof(VkCommandBuffer));

  // Create the render pass
  VkAttachmentDescription color_attachment = {0};
  color_attachment.flags = 0;
  color_attachment.format = image_format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachmentReference = {0};
  color_attachmentReference.attachment = 0;
  color_attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {0};
  subpass.flags = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = NULL;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachmentReference;
  subpass.pResolveAttachments = NULL;
  subpass.pDepthStencilAttachment = NULL;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = NULL;

  VkSubpassDependency subpass_dependency = {0};
  subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpass_dependency.dstSubpass = 0;
  subpass_dependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpass_dependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpass_dependency.srcAccessMask = 0;
  subpass_dependency.dstAccessMask = 0;
  subpass_dependency.dependencyFlags = 0;

  VkRenderPassCreateInfo render_pass_info = {0};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.pNext = NULL;
  render_pass_info.flags = 0;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &subpass_dependency;

  if (vkCreateRenderPass(device, &render_pass_info, NULL, &render_pass) !=
      VK_SUCCESS) {
    window_fail_with_error("An error occured while creating the render pass.");
  }

  // Create the command pool
  VkCommandPoolCreateInfo pool_info = {0};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.pNext = NULL;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = graphics_queue_id;

  if (vkCreateCommandPool(device, &pool_info, NULL, &command_pool) !=
      VK_SUCCESS) {
    window_fail_with_error("An error occured while creating the command pool;");
  }

  renderer_create_swapchain();

  // Create semaphores for rendering synchronization.
  VkSemaphoreCreateInfo semaphoreCreateInfo = {0};
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphoreCreateInfo.pNext = NULL;
  semaphoreCreateInfo.flags = 0;

  if (vkCreateSemaphore(device, &semaphoreCreateInfo, NULL,
                        &image_available_semaphore) != VK_SUCCESS ||
      vkCreateSemaphore(device, &semaphoreCreateInfo, NULL,
                        &render_finished_semaphore) != VK_SUCCESS) {
    window_fail_with_error(
        "An error occured while creating the rendering synchronization ");
  }
}

void renderer_quit() {
  vkQueueWaitIdle(graphics_queue);
  vkQueueWaitIdle(present_queue);
  vkDeviceWaitIdle(device);

  renderer_cleanup_swapchain();

  vkDestroyRenderPass(device, render_pass, NULL);

  free(command_buffers);

  vkDestroyCommandPool(device, command_pool, NULL);

  vkDestroySemaphore(device, render_finished_semaphore, NULL);
  vkDestroySemaphore(device, image_available_semaphore, NULL);

  free(framebuffers);
  free(swapchain_image_views);

  pipeline_cache_quit();

  vkDestroyDevice(device, NULL);

  vkDestroyInstance(instance, NULL);
}

void renderer_render() {
  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                                          image_available_semaphore,
                                          VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    renderer_cleanup_swapchain();
    renderer_create_swapchain();

    pipeline_destroy(*pipeline);
    pipeline_create(pipeline);

    renderer_record_command_buffers();

    return;
  }

  VkPipelineStageFlags waitStage =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submit_info;
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.pNext = NULL;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &image_available_semaphore;
  submit_info.pWaitDstStageMask = &waitStage;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffers[imageIndex];
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &render_finished_semaphore;

  if (vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE) !=
      VK_SUCCESS) {
    window_fail_with_error("Error submitting command buffers.");
  }

  VkPresentInfoKHR present_info;
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.pNext = NULL;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &render_finished_semaphore;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain;
  present_info.pImageIndices = &imageIndex;
  present_info.pResults = NULL;

  result = vkQueuePresentKHR(present_queue, &present_info);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    renderer_cleanup_swapchain();
    renderer_create_swapchain();

    pipeline_destroy(*pipeline);
    pipeline_create(pipeline);

    renderer_record_command_buffers();
  }

  vkQueueWaitIdle(present_queue);

  frame_id = (frame_id + 1) % image_count;
}

// Create the swapchain.
void renderer_create_swapchain() {
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
                                            &capabilities);

  // Choose the best resolution
  if (capabilities.currentExtent.width != UINT32_MAX) {
    image_extent = capabilities.currentExtent;
  } else {
    // Clamp the window size within the supported range
    image_extent.width = FLAP_WINDOW_WIDTH;
    image_extent.height = FLAP_WINDOW_HEIGHT;
    if (image_extent.width < capabilities.minImageExtent.width) {
      image_extent.width = capabilities.minImageExtent.width;
    } else if (image_extent.width > capabilities.maxImageExtent.width) {
      image_extent.width = capabilities.maxImageExtent.width;
    }

    if (image_extent.height < capabilities.minImageExtent.height) {
      image_extent.height = capabilities.minImageExtent.height;
    } else if (image_extent.height > capabilities.maxImageExtent.width) {
      image_extent.height = capabilities.maxImageExtent.width;
    }
  }

  VkSwapchainCreateInfoKHR swapchain_info = {0};
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.pNext = NULL;
  swapchain_info.flags = 0;
  swapchain_info.surface = surface;
  swapchain_info.minImageCount = image_count;
  swapchain_info.imageFormat = image_format;
  swapchain_info.imageColorSpace = image_color_space;
  swapchain_info.imageExtent = image_extent;
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  if (graphics_queue_id != present_queue_id) {
    uint32_t queueFamilyIndices[2] = {graphics_queue_id, present_queue_id};

    swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_info.queueFamilyIndexCount = 2;
    swapchain_info.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_info.queueFamilyIndexCount = 0;
    swapchain_info.pQueueFamilyIndices = NULL;
  }

  swapchain_info.preTransform = swapchain_pre_transform;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.presentMode = present_mode;
  swapchain_info.clipped = VK_TRUE;
  swapchain_info.oldSwapchain = swapchain;

  if (vkCreateSwapchainKHR(device, &swapchain_info, NULL, &swapchain) !=
      VK_SUCCESS) {
    window_fail_with_error("An error occured while creating the swapchain.");
  }

  // Retrieve the swapchain images.
  vkGetSwapchainImagesKHR(device, swapchain, &image_count, NULL);

  VkImage *swapchain_images = (VkImage *)malloc(image_count * sizeof(VkImage));
  swapchain_image_views =
      (VkImageView *)malloc(image_count * sizeof(VkImageView));

  vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images);

  for (uint32_t i = 0; i < image_count; ++i) {
    VkImageViewCreateInfo image_view_create_info = {0};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.pNext = NULL;
    image_view_create_info.flags = 0;
    image_view_create_info.image = swapchain_images[i];
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = image_format;
    image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &image_view_create_info, NULL,
                          &swapchain_image_views[i])

        != VK_SUCCESS) {
      window_fail_with_error(
          "An error occured while creating the swapchain image views.");
    }

    VkFramebufferCreateInfo framebuffer_info = {0};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.pNext = NULL;
    framebuffer_info.flags = 0;
    framebuffer_info.renderPass = render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = &swapchain_image_views[i];
    framebuffer_info.width = image_extent.width;
    framebuffer_info.height = image_extent.height;
    framebuffer_info.layers = 1;

    if (vkCreateFramebuffer(device, &framebuffer_info, NULL,
                            &framebuffers[i]) != VK_SUCCESS) {
      window_fail_with_error(
          "An error occured while creating the framebuffers.");
    }
  }

  free(swapchain_images);

  // Allocate the command buffers.
  VkCommandBufferAllocateInfo command_buffer_info = {0};
  command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_info.pNext = NULL;
  command_buffer_info.commandPool = command_pool;
  command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  command_buffer_info.commandBufferCount = image_count;

  if (vkAllocateCommandBuffers(device, &command_buffer_info, command_buffers) !=
      VK_SUCCESS) {
    window_fail_with_error(
        "An error occured while creating the command buffers.");
  }
}

void renderer_cleanup_swapchain() {
  vkDeviceWaitIdle(device);

  for (uint32_t i = 0; i < image_count; i++) {
    vkDestroyFramebuffer(device, framebuffers[i], NULL);
    vkDestroyImageView(device, swapchain_image_views[i], NULL);
  }

  vkFreeCommandBuffers(device, command_pool, image_count, command_buffers);

  vkDestroySwapchainKHR(device, swapchain, NULL);
  swapchain = VK_NULL_HANDLE;
}

VkDevice renderer_get_device() { return device; }

const VkExtent2D renderer_get_extent() { return image_extent; }

const VkRenderPass renderer_get_render_pass() { return render_pass; }

void renderer_set_pipeline(Pipeline *next_pipeline) {
  pipeline = next_pipeline;
}

void renderer_set_vertex_buffer(VkBuffer buffer) { vertex_buffer = buffer; }

void renderer_record_command_buffers() {
  if (pipeline == NULL) {
    window_fail_with_error(
        "Recording command buffers without a graphics pipeline.");
  } else if (vertex_buffer == VK_NULL_HANDLE) {
    window_fail_with_error(
        "Recording command buffers without a vertex buffer.");
  }

  VkCommandBufferBeginInfo begin_info = {0};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.pNext = NULL;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  begin_info.pInheritanceInfo = NULL;

  for (uint32_t i = 0; i < image_count; ++i) {
    vkBeginCommandBuffer(command_buffers[i], &begin_info);

    const VkClearValue clearColor = {{{0.53f, 0.81f, 0.92f, 1.f}}};

    VkRenderPassBeginInfo renderPassBeginInfo = {0};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = NULL;
    renderPassBeginInfo.renderPass = render_pass;
    renderPassBeginInfo.framebuffer = framebuffers[i];
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = image_extent;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(command_buffers[i], &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline->pipeline);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(command_buffers[i], 0, 1, &vertex_buffer, &offset);

    // Draw player
    vkCmdPushConstants(command_buffers[i], pipeline->pipeline_layout,
                       VK_SHADER_STAGE_FRAGMENT_BIT, 0, 3 * sizeof(float),
                       FLAP_BIRD_COLOR);
    vkCmdDraw(command_buffers[i], 1, 1, 0, 0);

    // Draw pipes
    vkCmdPushConstants(command_buffers[i], pipeline->pipeline_layout,
                       VK_SHADER_STAGE_FRAGMENT_BIT, 0, 3 * sizeof(float),
                       FLAP_PIPE_COLOR);
    vkCmdDraw(command_buffers[i], FLAP_NUM_PIPES * 2, 1, 1, 0);

    vkCmdEndRenderPass(command_buffers[i]);

    vkEndCommandBuffer(command_buffers[i]);
  }
}

void renderer_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags memoryProperties,
                            VkBuffer *buffer, VkDeviceMemory *bufferMemory) {
  VkBufferCreateInfo bufferCreateInfo = {0};
  bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCreateInfo.pNext = NULL;
  bufferCreateInfo.flags = 0;
  bufferCreateInfo.size = size;
  bufferCreateInfo.usage = usage;
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  bufferCreateInfo.queueFamilyIndexCount = 0;
  bufferCreateInfo.pQueueFamilyIndices = NULL;

  vkCreateBuffer(device, &bufferCreateInfo, NULL, buffer);

  VkMemoryRequirements memoryRequirements;
  vkGetBufferMemoryRequirements(device, *buffer, &memoryRequirements);

  uint32_t bestMemory = 0;
  VkBool32 memoryFound = VK_FALSE;
  for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount;
       i++) {
    VkMemoryType memType = physical_device_memory_properties.memoryTypes[i];
    if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
        (memType.propertyFlags & memoryProperties)) {
      bestMemory = i;
      memoryFound = VK_TRUE;
      break;
    }
  }
  if (memoryFound == VK_FALSE) {
    window_fail_with_error(
        "Allocation failed: no suitable memory type could be found!");
  }

  VkMemoryAllocateInfo allocate_info = {0};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.pNext = NULL;
  allocate_info.allocationSize = memoryRequirements.size;
  allocate_info.memoryTypeIndex = bestMemory;

  vkAllocateMemory(device, &allocate_info, NULL, bufferMemory);

  vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

void renderer_buffer_data(VkDeviceMemory buffer_memory, VkDeviceSize size,
                          const void *data) {
  uint8_t *dest_memory;
  if (vkMapMemory(device, buffer_memory, 0, size, 0, (void *)&dest_memory) !=
      VK_SUCCESS) {
    window_fail_with_error("An error occured while mapping device memory.");
  }
  memcpy(dest_memory, data, (size_t)size);
  vkUnmapMemory(device, buffer_memory);
}
