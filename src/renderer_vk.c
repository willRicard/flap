#include "renderer_vk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flap.h"
#include "pipeline_vk.h"
#include "window_vk.h"

// Vulkan context
static VkInstance instance = VK_NULL_HANDLE;

static VkDevice device = VK_NULL_HANDLE;
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
static VkImage *swapchain_images = NULL;
static VkImageView *swapchain_image_views = NULL;

// Render data
static VkRenderPass render_pass = VK_NULL_HANDLE;

static VkFramebuffer *framebuffers = NULL;

static VkCommandPool command_pool = VK_NULL_HANDLE;
static VkCommandBuffer *command_buffers = NULL;

static VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
static VkSemaphore render_finished_semaphore = VK_NULL_HANDLE;

static VkBuffer vertex_buffer = VK_NULL_HANDLE;
static VkBuffer index_buffer = VK_NULL_HANDLE;

static Pipeline *pipeline = NULL;
static VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

void renderer_init() {
  // Create the instance
  uint32_t surface_extension_count = 0;
  const char **surface_extensions =
      vulkan_window_get_extensions(&surface_extension_count);

  VkInstance instance;

  VkApplicationInfo appInfo;
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = NULL;
  appInfo.pApplicationName = "Flap";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo instanceCreateInfo;
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pNext = NULL;
  instanceCreateInfo.flags = 0;
  instanceCreateInfo.pApplicationInfo = &appInfo;
  instanceCreateInfo.enabledLayerCount = 0;
  instanceCreateInfo.ppEnabledLayerNames = NULL;
  instanceCreateInfo.enabledExtensionCount = surface_extension_count;
  instanceCreateInfo.ppEnabledExtensionNames = surface_extensions;

  VkResult result = vkCreateInstance(&instanceCreateInfo, NULL, &instance);
  if (result != VK_SUCCESS) {
    window_fail_with_error(
        "An error occured while creating the Vulkan instance.");
  }

  uint32_t deviceCount = 0;
  if (vkEnumeratePhysicalDevices(instance, &deviceCount, NULL) != VK_SUCCESS) {
    window_fail_with_error("An error occured while enumerating devices.");
  }

  VkPhysicalDevice *devices =
      (VkPhysicalDevice *)malloc(deviceCount * sizeof(VkPhysicalDevice));

  if (vkEnumeratePhysicalDevices(instance, &deviceCount, devices) !=
      VK_SUCCESS) {
    window_fail_with_error("An error occured while enumerating devices.");
  }

  // Create the device.
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  for (uint32_t i = 0; i < deviceCount; i++) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(devices[i], &properties);
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      physicalDevice = devices[i];
      break;
    } else if (properties.deviceType ==
               VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
      physicalDevice = devices[i];
    }
  }
  if (physicalDevice == VK_NULL_HANDLE) {
    window_fail_with_error("No suitable device was found.");
  }

  free(devices);

  // Query memory properties for later allocations
  vkGetPhysicalDeviceMemoryProperties(physicalDevice,
                                      &physical_device_memory_properties);

  // Create the window surface
  if (vulkan_window_create_surface(instance, &surface) != VK_SUCCESS) {
    window_fail_with_error(
        "An error occured while creating the window surface.");
  }

  // Pick the best graphics & present queues

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           NULL);

  VkQueueFamilyProperties *queueFamilyProperties =
      (VkQueueFamilyProperties *)malloc(queueFamilyCount *
                                        sizeof(VkQueueFamilyProperties));

  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
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
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface,
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

  VkDeviceQueueCreateInfo queueCreateInfos[2];

  queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfos[0].pNext = NULL;
  queueCreateInfos[0].flags = 0;
  queueCreateInfos[0].queueFamilyIndex = graphics_queue_id;
  queueCreateInfos[0].queueCount = 1;
  queueCreateInfos[0].pQueuePriorities = &priority;

  queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfos[1].pNext = NULL;
  queueCreateInfos[1].flags = 0;
  queueCreateInfos[1].queueFamilyIndex = present_queue_id;
  queueCreateInfos[1].queueCount = 1;
  queueCreateInfos[1].pQueuePriorities = &priority;

  static const char *deviceExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  VkDeviceCreateInfo deviceCreateInfo = {0};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pNext = NULL;
  deviceCreateInfo.flags = 0;
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
  deviceCreateInfo.enabledLayerCount = 0;
  deviceCreateInfo.ppEnabledLayerNames = NULL;
  deviceCreateInfo.enabledExtensionCount = 1;
  deviceCreateInfo.ppEnabledExtensionNames = &deviceExtension;
  deviceCreateInfo.pEnabledFeatures = NULL;

  vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device);

  vkGetDeviceQueue(device, graphics_queue_id, 0, &graphics_queue);
  vkGetDeviceQueue(device, present_queue_id, 0, &present_queue);

  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                            &capabilities);

  // Pick the best image format
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                       NULL);

  VkSurfaceFormatKHR *formats =
      (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));

  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
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
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                            &presentModeCount, NULL);

  VkPresentModeKHR *presentModes =
      (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));

  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
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

  // Choose the best image count
  if (image_count < capabilities.minImageCount) {
    image_count = capabilities.minImageCount;
  } else if (image_count > capabilities.maxImageCount) {
    image_count = capabilities.maxImageCount;
  }

  renderer_create_swapchain();

  // Retrieve the swapchain images.
  vkGetSwapchainImagesKHR(device, swapchain, &image_count, NULL);

  swapchain_images = (VkImage *)malloc(image_count * sizeof(VkImage));
  swapchain_image_views =
      (VkImageView *)malloc(image_count * sizeof(VkImageView));

  vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images);

  for (uint32_t i = 0; i < image_count; ++i) {
    VkImageViewCreateInfo imageViewCreateInfo = {0};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext = NULL;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.image = swapchain_images[i];
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = image_format;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &imageViewCreateInfo, NULL,
                          &swapchain_image_views[i])

        != VK_SUCCESS) {
      window_fail_with_error(
          "An error occured while creating the swapchain image views.");
    }
  }

  // Create the render pass
  VkAttachmentDescription colorAttachment = {0};
  colorAttachment.flags = 0;
  colorAttachment.format = image_format;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentReference = {0};
  colorAttachmentReference.attachment = 0;
  colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {0};
  subpass.flags = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = NULL;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentReference;
  subpass.pResolveAttachments = NULL;
  subpass.pDepthStencilAttachment = NULL;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = NULL;

  VkSubpassDependency subpassDependency = {0};
  subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependency.dstSubpass = 0;
  subpassDependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcAccessMask = 0;
  subpassDependency.dstAccessMask = 0;
  subpassDependency.dependencyFlags = 0;

  VkRenderPassCreateInfo renderPassCreateInfo = {0};
  renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassCreateInfo.pNext = NULL;
  renderPassCreateInfo.flags = 0;
  renderPassCreateInfo.attachmentCount = 1;
  renderPassCreateInfo.pAttachments = &colorAttachment;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpass;
  renderPassCreateInfo.dependencyCount = 1;
  renderPassCreateInfo.pDependencies = &subpassDependency;

  if (vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &render_pass) !=
      VK_SUCCESS) {
    window_fail_with_error("An error occured while creating the render pass.");
  }

  framebuffers = (VkFramebuffer *)malloc(image_count * sizeof(VkFramebuffer));
  for (uint32_t i = 0; i < image_count; i++) {
    VkFramebufferCreateInfo framebufferCreateInfo = {0};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = NULL;
    framebufferCreateInfo.flags = 0;
    framebufferCreateInfo.renderPass = render_pass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = &swapchain_image_views[i];
    framebufferCreateInfo.width = image_extent.width;
    framebufferCreateInfo.height = image_extent.height;
    framebufferCreateInfo.layers = 1;

    if (vkCreateFramebuffer(device, &framebufferCreateInfo, NULL,
                            &framebuffers[i]) != VK_SUCCESS) {
      window_fail_with_error(
          "An error occured while creating the framebuffers.");
    }
  }

  // Create the command pool
  VkCommandPoolCreateInfo commandPoolCreateInfo = {0};
  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.pNext = NULL;
  commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  commandPoolCreateInfo.queueFamilyIndex = graphics_queue_id;

  if (vkCreateCommandPool(device, &commandPoolCreateInfo, NULL,
                          &command_pool) != VK_SUCCESS) {
    window_fail_with_error("An error occured while creating the command pool;");
  }

  // Allocate the command buffers.
  VkCommandBufferAllocateInfo commandBufferAllocateInfo = {0};
  commandBufferAllocateInfo.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocateInfo.pNext = NULL;
  commandBufferAllocateInfo.commandPool = command_pool;
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandBufferCount = image_count;

  command_buffers =
      (VkCommandBuffer *)malloc(image_count * sizeof(VkCommandBuffer));

  if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo,
                               command_buffers) != VK_SUCCESS) {
    window_fail_with_error(
        "An error occured while creating the command buffers.");
  }

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

  vkFreeCommandBuffers(device, command_pool, image_count, command_buffers);
  free(command_buffers);

  vkDestroyCommandPool(device, command_pool, NULL);

  vkDestroySemaphore(device, render_finished_semaphore, NULL);
  vkDestroySemaphore(device, image_available_semaphore, NULL);

  for (uint32_t i = 0; i < image_count; i++) {
    vkDestroyFramebuffer(device, framebuffers[i], NULL);
    vkDestroyImageView(device, swapchain_image_views[i], NULL);
  }

  free(framebuffers);
  free(swapchain_image_views);
  free(swapchain_images);

  vkDestroyRenderPass(device, render_pass, NULL);

  vkDestroySwapchainKHR(device, swapchain, NULL);

  vkDestroyDevice(device, NULL);

  vkDestroyInstance(instance, NULL);
}

void renderer_render() {
  uint32_t imageIndex;
  vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                        image_available_semaphore, VK_NULL_HANDLE, &imageIndex);

  VkPipelineStageFlags waitStage =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submitInfo;
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &image_available_semaphore;
  submitInfo.pWaitDstStageMask = &waitStage;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command_buffers[imageIndex];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &render_finished_semaphore;

  if (vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE) !=
      VK_SUCCESS) {
    window_fail_with_error("Error submitting command buffers.");
  }

  VkPresentInfoKHR presentInfo;
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = NULL;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &render_finished_semaphore;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swapchain;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = NULL;

  vkQueuePresentKHR(present_queue, &presentInfo);

  vkQueueWaitIdle(present_queue);

  frame_id = (frame_id + 1) % image_count;
}

// Create the swapchain.
void renderer_create_swapchain() {
  VkSwapchainCreateInfoKHR swapchainCreateInfo = {0};
  swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.pNext = NULL;
  swapchainCreateInfo.flags = 0;
  swapchainCreateInfo.surface = surface;
  swapchainCreateInfo.minImageCount = image_count;
  swapchainCreateInfo.imageFormat = image_format;
  swapchainCreateInfo.imageColorSpace = image_color_space;
  swapchainCreateInfo.imageExtent = image_extent;
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  if (graphics_queue_id != present_queue_id) {
    uint32_t queueFamilyIndices[2] = {graphics_queue_id, present_queue_id};

    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchainCreateInfo.queueFamilyIndexCount = 2;
    swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = NULL;
  }

  swapchainCreateInfo.preTransform = swapchain_pre_transform;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
  swapchainCreateInfo.presentMode = present_mode;
  swapchainCreateInfo.clipped = VK_TRUE;
  swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, &swapchain) !=
      VK_SUCCESS) {
    window_fail_with_error("An error occured while creating the swapchain.");
  }
}

void renderer_cleanup_swapchain() {

}

VkDevice renderer_get_device() { return device; }

const VkExtent2D renderer_get_extent() { return image_extent; }

const VkRenderPass renderer_get_render_pass() { return render_pass; }

void renderer_set_pipeline(Pipeline *next_pipeline) {
  pipeline = next_pipeline;
}

void renderer_set_vertex_buffer(VkBuffer buffer) { vertex_buffer = buffer; }

void renderer_set_index_buffer(VkBuffer buffer) { index_buffer = buffer; }

void renderer_record_command_buffers() {
  if (pipeline == NULL) {
    window_fail_with_error(
        "Recording command buffers without a graphics pipeline.");
  } else if (vertex_buffer == VK_NULL_HANDLE) {
    window_fail_with_error(
        "Recording command buffers without a vertex buffer.");
  } else if (index_buffer == VK_NULL_HANDLE) {
    window_fail_with_error(
        "Recording command buffers without an index buffer.");
  }

  VkCommandBufferBeginInfo beginInfo = {0};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.pNext = NULL;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  beginInfo.pInheritanceInfo = NULL;

  for (uint32_t i = 0; i < image_count; ++i) {
    vkBeginCommandBuffer(command_buffers[i], &beginInfo);

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

    vkCmdBindIndexBuffer(command_buffers[i], index_buffer, 0,
                         VK_INDEX_TYPE_UINT16);

    // Draw player
    vkCmdPushConstants(command_buffers[i], pipeline->pipeline_layout,
                       VK_SHADER_STAGE_FRAGMENT_BIT, 0, 3 * sizeof(float),
                       FLAP_BIRD_COLOR);
    vkCmdDrawIndexed(command_buffers[i], 6, 1, 0, 0, 0);

    // Draw pipes
    vkCmdPushConstants(command_buffers[i], pipeline->pipeline_layout,
                       VK_SHADER_STAGE_FRAGMENT_BIT, 0, 3 * sizeof(float),
                       FLAP_PIPE_COLOR);
    vkCmdDrawIndexed(command_buffers[i], (FLAP_NUM_PIPES * 2) * 6, 1, 6, 0, 0);

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

  VkMemoryAllocateInfo allocateInfo = {0};
  allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocateInfo.pNext = NULL;
  allocateInfo.allocationSize = memoryRequirements.size;
  allocateInfo.memoryTypeIndex = bestMemory;

  vkAllocateMemory(device, &allocateInfo, NULL, bufferMemory);

  vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

void renderer_buffer_data(VkDeviceMemory bufferMemory, VkDeviceSize size,
                          const void *data) {
  uint8_t *destMemory;
  if (vkMapMemory(device, bufferMemory, 0, size, 0, (void *)&destMemory) !=
      VK_SUCCESS) {
    window_fail_with_error("An error occured while mapping device memory.");
  }
  memcpy(destMemory, data, (size_t)size);
  vkUnmapMemory(device, bufferMemory);
}
