#include "renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "flap.h"
#include "pipeline.h"
#include "window.h"

// Vulkan context
static VkInstance instance = VK_NULL_HANDLE;

static VkDevice device = VK_NULL_HANDLE;
static VkPhysicalDevice physical_device = VK_NULL_HANDLE;
static VkPhysicalDeviceProperties physical_device_properties = {0};
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

void renderer_init() {
  // Create the instance
  uint32_t surface_extension_count = 0;
  const char **surface_extensions =
      window_get_extensions(&surface_extension_count);

  VkInstance instance;

  VkApplicationInfo app_info = {0};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Flap";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo instance_info = {0};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledExtensionCount = surface_extension_count;
  instance_info.ppEnabledExtensionNames = surface_extensions;

  VK_CHECK(vkCreateInstance(&instance_info, NULL, &instance),
           "An error occured while creating the Vulkan instance.")

  uint32_t device_count = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(instance, &device_count, NULL),
           "vkEnumeratePhysicalDevices");

  VkPhysicalDevice *devices =
      (VkPhysicalDevice *)malloc(device_count * sizeof(VkPhysicalDevice));

  VK_CHECK(vkEnumeratePhysicalDevices(instance, &device_count, devices),
           "An error occured while enumerating devices.")

  // Create the device.
  for (uint32_t i = 0; i < device_count; i++) {
    vkGetPhysicalDeviceProperties(devices[i], &physical_device_properties);
    if (physical_device_properties.deviceType ==
        VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      physical_device = devices[i];
      break;
    } else if (physical_device_properties.deviceType ==
               VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
      physical_device = devices[i];
    }
  }
  if (physical_device == VK_NULL_HANDLE) {
    fail_with_error("No suitable device was found.");
  }

  free(devices);

  // Query memory properties for later allocations
  vkGetPhysicalDeviceMemoryProperties(physical_device,
                                      &physical_device_memory_properties);

  // Create the window surface
  VK_CHECK(window_create_surface(instance, &surface),
           "An error occured while creating the window surface.")

  // Pick the best graphics & present queues
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count,
                                           NULL);

  VkQueueFamilyProperties *queue_family_properties =
      (VkQueueFamilyProperties *)malloc(queue_family_count *
                                        sizeof(VkQueueFamilyProperties));

  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count,
                                           queue_family_properties);

  for (uint32_t i = 0; i < queue_family_count; i++) {
    VkQueueFamilyProperties queue_family = queue_family_properties[i];
    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphics_queue_id = i;
      break;
    } else if (i == queue_family_count - 1) {
      fail_with_error("No graphics queue was found.");
    }
  }
  for (uint32_t i = 0; i < queue_family_count; i++) {
    VkBool32 present_supported;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                                         &present_supported);
    if (present_supported) {
      present_queue_id = i;
      break;
    } else if (i == queue_family_count - 1) {
      fail_with_error("No present queue was found.");
    }
  }

  free(queue_family_properties);

  float priority = 1.f;

  VkDeviceQueueCreateInfo queue_infos[2] = {{0}, {0}};

  queue_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_infos[0].queueFamilyIndex = graphics_queue_id;
  queue_infos[0].queueCount = 1;
  queue_infos[0].pQueuePriorities = &priority;

  queue_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_infos[1].queueFamilyIndex = present_queue_id;
  queue_infos[1].queueCount = 1;
  queue_infos[1].pQueuePriorities = &priority;

  VkPhysicalDeviceFeatures features = {0};
  vkGetPhysicalDeviceFeatures(physical_device, &features);

  static const char *deviceExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  VkDeviceCreateInfo device_info = {0};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = NULL;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = queue_infos;
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
  uint32_t present_mode_count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                            &present_mode_count, NULL);

  VkPresentModeKHR *present_modes =
      (VkPresentModeKHR *)malloc(present_mode_count * sizeof(VkPresentModeKHR));

  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                            &present_mode_count, present_modes);

  for (uint32_t i = 0; i < present_mode_count; i++) {
    if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
      break;
    } else if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
  }

  free(present_modes);

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
  color_attachment.format = image_format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_reference = {0};
  color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {0};
  subpass.flags = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_reference;

  VkSubpassDependency subpass_dependency = {0};
  subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpass_dependency.dstSubpass = 0;
  subpass_dependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpass_dependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkRenderPassCreateInfo render_pass_info = {0};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &subpass_dependency;

  VK_CHECK(vkCreateRenderPass(device, &render_pass_info, NULL, &render_pass),
           "An error occured while creating the render pass.")

  // Create the command pool
  VkCommandPoolCreateInfo pool_info = {0};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = graphics_queue_id;

  VK_CHECK(vkCreateCommandPool(device, &pool_info, NULL, &command_pool),
           "An error occured while creating the command pool;");

  renderer_create_swapchain();

  // Create semaphores for rendering synchronization.
  VkSemaphoreCreateInfo semaphore_info = {0};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VK_CHECK(vkCreateSemaphore(device, &semaphore_info, NULL,
                             &image_available_semaphore),
           "An error occured while creating the 'image available' semaphore.")
  VK_CHECK(vkCreateSemaphore(device, &semaphore_info, NULL,
                             &render_finished_semaphore),
           "An error occured while creating the 'render finished' semaphore.")
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

    return;
  }

  VkPipelineStageFlags waitStage =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submit_info = {0};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &image_available_semaphore;
  submit_info.pWaitDstStageMask = &waitStage;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffers[imageIndex];
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &render_finished_semaphore;

  VK_CHECK(vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE),
           "Error submitting command buffers.")

  VkPresentInfoKHR present_info = {0};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &render_finished_semaphore;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain;
  present_info.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(present_queue, &present_info);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    renderer_cleanup_swapchain();
    renderer_create_swapchain();
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
  swapchain_info.surface = surface;
  swapchain_info.minImageCount = image_count;
  swapchain_info.imageFormat = image_format;
  swapchain_info.imageColorSpace = image_color_space;
  swapchain_info.imageExtent = image_extent;
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  if (graphics_queue_id != present_queue_id) {
    uint32_t queue_familyIndices[2] = {graphics_queue_id, present_queue_id};

    swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_info.queueFamilyIndexCount = 2;
    swapchain_info.pQueueFamilyIndices = queue_familyIndices;
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

  VK_CHECK(vkCreateSwapchainKHR(device, &swapchain_info, NULL, &swapchain),
           "An error occured while creating the swapchain.")

  // Retrieve the swapchain images.
  vkGetSwapchainImagesKHR(device, swapchain, &image_count, NULL);

  VkImage *swapchain_images = (VkImage *)malloc(image_count * sizeof(VkImage));
  swapchain_image_views =
      (VkImageView *)malloc(image_count * sizeof(VkImageView));

  vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images);

  for (uint32_t i = 0; i < image_count; ++i) {
    VkImageViewCreateInfo image_view_create_info = {0};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
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

    VK_CHECK(vkCreateImageView(device, &image_view_create_info, NULL,
                               &swapchain_image_views[i]),
             "An error occured while creating the swapchain image views.")

    VkFramebufferCreateInfo framebuffer_info = {0};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = &swapchain_image_views[i];
    framebuffer_info.width = image_extent.width;
    framebuffer_info.height = image_extent.height;
    framebuffer_info.layers = 1;

    VK_CHECK(
        vkCreateFramebuffer(device, &framebuffer_info, NULL, &framebuffers[i]),
        "An error occured while creating the framebuffers.")
  }

  free(swapchain_images);

  // Allocate the command buffers.
  VkCommandBufferAllocateInfo command_buffer_info = {0};
  command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_info.commandPool = command_pool;
  command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  command_buffer_info.commandBufferCount = image_count;

  VK_CHECK(
      vkAllocateCommandBuffers(device, &command_buffer_info, command_buffers),
      "An error occured while creating the command buffers.")
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

VkPhysicalDeviceProperties renderer_get_physical_device_properties() {
  return physical_device_properties;
}

VkPhysicalDeviceMemoryProperties
renderer_get_physical_device_memory_properties() {
  return physical_device_memory_properties;
}

uint32_t renderer_get_image_count() { return image_count; }

const VkExtent2D renderer_get_extent() { return image_extent; }

VkRenderPass renderer_get_render_pass() { return render_pass; }

VkCommandBuffer *renderer_begin_command_buffer() {
  VkCommandBuffer *command_buffer =
      (VkCommandBuffer *)malloc(sizeof(VkCommandBuffer));

  VkCommandBufferAllocateInfo command_buffer_info = {0};
  command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_info.commandPool = command_pool;
  command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  command_buffer_info.commandBufferCount = 1;

  VK_CHECK(
      vkAllocateCommandBuffers(device, &command_buffer_info, command_buffer),
      "An error occured while creating the command buffers.")

  VkCommandBufferBeginInfo begin_info = {0};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(*command_buffer, &begin_info);

  return command_buffer;
}

void renderer_end_command_buffer(VkCommandBuffer *command_buffer) {
  vkEndCommandBuffer(*command_buffer);

  VkSubmitInfo submit_info = {0};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = command_buffer;

  VK_CHECK(vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE),
           "Error submitting (one-time) command buffers.")
  vkQueueWaitIdle(graphics_queue);

  vkFreeCommandBuffers(device, command_pool, 1, command_buffer);
  free(command_buffer);
}

VkCommandBuffer *renderer_begin_command_buffers(uint32_t *buffer_count) {
  *buffer_count = image_count;

  VkCommandBufferBeginInfo begin_info = {0};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  for (uint32_t i = 0; i < image_count; ++i) {
    vkBeginCommandBuffer(command_buffers[i], &begin_info);

    const VkClearValue clearColor = {{{0.53f, 0.81f, 0.92f, 1.f}}};

    VkRenderPassBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin_info.renderPass = render_pass;
    begin_info.framebuffer = framebuffers[i];
    begin_info.renderArea.extent = image_extent;
    begin_info.clearValueCount = 1;
    begin_info.pClearValues = &clearColor;

    vkCmdBeginRenderPass(command_buffers[i], &begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);
  }
  return command_buffers;
}

void renderer_end_command_buffers() {
  for (uint32_t i = 0; i < image_count; i++) {
    vkCmdEndRenderPass(command_buffers[i]);
    vkEndCommandBuffer(command_buffers[i]);
  }
}
