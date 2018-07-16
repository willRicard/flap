#include "renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flap.h"
#include "window.h"

const float FLAP_BIRD_COLOR[] = {1.f, 1.f, 0.f};
const float FLAP_PIPE_COLOR[] = {0.035f, 0.42f, 0.035f};

static VkInstance instance = VK_NULL_HANDLE;

static VkDevice device = VK_NULL_HANDLE;
static VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;

static VkSurfaceKHR surface = VK_NULL_HANDLE;

static VkQueue graphicsQueue, presentQueue;

static VkSwapchainKHR swapchain = VK_NULL_HANDLE;

static VkFormat imageFormat = VK_FORMAT_UNDEFINED;
static VkExtent2D imageExtent;

static uint32_t imageCount = 3;
static VkImage *swapchainImages;
static VkImageView *swapchainImageViews;

static VkRenderPass renderPass;

static VkFramebuffer *framebuffers;

static VkCommandPool commandPool = VK_NULL_HANDLE;
static VkCommandBuffer *commandBuffers;

static VkSemaphore imageAvailableSemaphore, renderFinishedSemaphore;

static VkBuffer vertexBuffer = VK_NULL_HANDLE;
static VkBuffer indexBuffer = VK_NULL_HANDLE;

static VkPipeline pipeline;
static VkPipelineLayout pipelineLayout;

void flapRendererInit() {
  // Create the instance
  uint32_t surfaceExtentionCount = 0;
  const char **surfaceExtensions =
      flapWindowGetExtensions(&surfaceExtentionCount);

  VkInstance instance;

  VkApplicationInfo appInfo = {};
  appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pApplicationName = "Flap";
  appInfo.pEngineName = "No Engine";

  VkInstanceCreateInfo instanceCreateInfo = {};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pApplicationInfo = &appInfo;
  instanceCreateInfo.enabledExtensionCount = surfaceExtentionCount;
  instanceCreateInfo.ppEnabledExtensionNames = surfaceExtensions;

  VkResult result = vkCreateInstance(&instanceCreateInfo, NULL, &instance);
  if (result != VK_SUCCESS) {
    fputs("An error occured while creating the Vulkan instance.", stderr);
    exit(EXIT_FAILURE);
  }

  uint32_t deviceCount = 0;
  if (vkEnumeratePhysicalDevices(instance, &deviceCount, NULL) != VK_SUCCESS) {
    fputs("An error occured while enumerating devices.", NULL);
    exit(EXIT_FAILURE);
  }

  VkPhysicalDevice *devices =
      (VkPhysicalDevice *)malloc(deviceCount * sizeof(VkPhysicalDevice));

  if (vkEnumeratePhysicalDevices(instance, &deviceCount, devices) !=
      VK_SUCCESS) {
    fputs("An error occured while enumerating devices.", NULL);
    exit(EXIT_FAILURE);
  }

  // Create the device.
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  for (uint32_t i = 0; i < deviceCount; i++) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(devices[i], &properties);
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
      physicalDevice = devices[i];
      break;
    } else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      physicalDevice = devices[i];
    }
  }
  if (physicalDevice == VK_NULL_HANDLE) {
    fputs("No suitable device was found.", stderr);
    exit(EXIT_FAILURE);
  }

  free(devices);

  // Query memory properties for later allocations, etc.
  vkGetPhysicalDeviceMemoryProperties(physicalDevice,
                                      &physicalDeviceMemoryProperties);

  if (flapWindowCreateSurface(instance, &surface) != VK_SUCCESS) {
    fputs("An error occured while creating the window surface.", stderr);
    exit(EXIT_FAILURE);
  }

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           NULL);

  VkQueueFamilyProperties *queueFamilyProperties =
      (VkQueueFamilyProperties *)malloc(queueFamilyCount *
                                        sizeof(VkQueueFamilyProperties));

  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           queueFamilyProperties);

  // Pick the first available graphics & present queues.
  uint32_t graphicsQueueID = 0, presentQueueID = 0;
  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    VkQueueFamilyProperties queueFamily = queueFamilyProperties[i];
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphicsQueueID = i;
      break;
    } else if (i == queueFamilyCount - 1) {
      fputs("No graphics queue was found.", stderr);
      exit(EXIT_FAILURE);
    }
  }
  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    VkBool32 presentSupported;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface,
                                         &presentSupported);
    if (presentSupported) {
      presentQueueID = i;
    } else if (i == queueFamilyCount - 1) {
      fputs("No present queue was found.", stderr);
      exit(EXIT_FAILURE);
    }
  }

  free(queueFamilyProperties);

  float priority = 1.f;

  VkDeviceQueueCreateInfo queueCreateInfos[2] = {};

  queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfos[0].queueFamilyIndex = graphicsQueueID;
  queueCreateInfos[0].queueCount = 1;
  queueCreateInfos[0].pQueuePriorities = &priority;

  queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfos[1].queueFamilyIndex = presentQueueID;
  queueCreateInfos[1].queueCount = 1;
  queueCreateInfos[1].pQueuePriorities = &priority;

  static const char *deviceExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
  deviceCreateInfo.enabledExtensionCount = 1;
  deviceCreateInfo.ppEnabledExtensionNames = &deviceExtension;

  vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device);

  vkGetDeviceQueue(device, graphicsQueueID, 0, &graphicsQueue);
  vkGetDeviceQueue(device, presentQueueID, 0, &presentQueue);

  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                            &capabilities);
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                       NULL);

  VkSurfaceFormatKHR *formats =
      (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));

  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                       formats);

  uint32_t presentModeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                            &presentModeCount, NULL);

  VkPresentModeKHR *presentModes =
      (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));

  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                            &presentModeCount, presentModes);

  VkSurfaceFormatKHR bestFormat;
  if (formatCount == 1 &&
      formats[0].format ==
          VK_FORMAT_UNDEFINED) { // We are free to define the format
    bestFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    bestFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  } else {
    bestFormat = formats[0];
  }
  imageFormat = bestFormat.format;

  // Choose the best present mode (see the spec, preference in descending
  // order).
  // 1. Mailbox
  // 2. Immediate
  // 3. FIFO
  VkPresentModeKHR bestPresentMode = VK_PRESENT_MODE_FIFO_KHR;
  for (uint32_t i = 0; i < presentModeCount; i++) {
    if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      bestPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
      break;
    } else if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      bestPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
  }

  // Choose the best resolution
  if (capabilities.currentExtent.width == UINT32_MAX) {
    imageExtent = capabilities.currentExtent;
  } else {
    // Clamp the window size within the supported range
    imageExtent.width = FLAP_WINDOW_WIDTH;
    imageExtent.height = FLAP_WINDOW_HEIGHT;
    if (imageExtent.width < capabilities.minImageExtent.width) {
      imageExtent.width = capabilities.minImageExtent.width;
    } else if (imageExtent.width > capabilities.maxImageExtent.width) {
      imageExtent.width = capabilities.maxImageExtent.width;
    }

    if (imageExtent.height < capabilities.minImageExtent.height) {
      imageExtent.height = capabilities.minImageExtent.height;
    } else if (imageExtent.height > capabilities.maxImageExtent.width) {
      imageExtent.height = capabilities.maxImageExtent.width;
    }
  }

  // Choose the best image count
  if (imageCount < capabilities.minImageCount) {
    imageCount = capabilities.minImageCount;
  } else if (imageCount > capabilities.maxImageCount) {
    imageCount = capabilities.maxImageCount;
  }

  free(presentModes);
  free(formats);

  // Create the swapchain.
  VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
  swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.surface = surface;
  swapchainCreateInfo.minImageCount = imageCount;
  swapchainCreateInfo.imageFormat = bestFormat.format;
  swapchainCreateInfo.imageColorSpace = bestFormat.colorSpace;
  swapchainCreateInfo.imageExtent = imageExtent;
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  if (graphicsQueueID != presentQueueID) {
    uint32_t queueFamilyIndices[2] = {graphicsQueueID, presentQueueID};

    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchainCreateInfo.queueFamilyIndexCount = 2;
    swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  swapchainCreateInfo.preTransform = capabilities.currentTransform;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode = bestPresentMode;
  swapchainCreateInfo.clipped = VK_TRUE;

  if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, &swapchain) !=
      VK_SUCCESS) {
    fputs("An error occured while creating the swapchain.", stderr);
    exit(EXIT_FAILURE);
  }

  // Retrieve the swapchain images.
  vkGetSwapchainImagesKHR(device, swapchain, &imageCount, NULL);

  swapchainImages = (VkImage *)malloc(imageCount * sizeof(VkImage));
  swapchainImageViews = (VkImageView *)malloc(imageCount * sizeof(VkImageView));

  vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages);

  for (uint32_t i = 0; i < imageCount; ++i) {
    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = swapchainImages[i];
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = imageFormat;
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
                          &swapchainImageViews[i])

        != VK_SUCCESS) {
      fputs("An error occured while creating the swapchain image views.",
            stderr);
      exit(EXIT_FAILURE);
    }
  }

  // Create the command pool
  VkCommandPoolCreateInfo commandPoolCreateInfo = {};
  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  commandPoolCreateInfo.queueFamilyIndex = graphicsQueueID;

  if (vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool) !=
      VK_SUCCESS) {
    fputs("An error occured while creating the command pool;", stderr);
    exit(EXIT_FAILURE);
  }

  // Create the render pass
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = imageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentReference = {};
  colorAttachmentReference.attachment = 0;
  colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentReference;

  VkRenderPassCreateInfo renderPassCreateInfo = {};
  renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassCreateInfo.attachmentCount = 1;
  renderPassCreateInfo.pAttachments = &colorAttachment;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpass;

  if (vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &renderPass) !=
      VK_SUCCESS) {
    fputs("An error occured while creating the render pass.", stderr);
    exit(EXIT_FAILURE);
  }

  framebuffers = (VkFramebuffer *)malloc(imageCount * sizeof(VkFramebuffer));
  for (uint32_t i = 0; i < imageCount; i++) {
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = &swapchainImageViews[i];
    framebufferCreateInfo.width = imageExtent.width;
    framebufferCreateInfo.height = imageExtent.height;
    framebufferCreateInfo.layers = 1;

    if (vkCreateFramebuffer(device, &framebufferCreateInfo, NULL,
                            &framebuffers[i]) != VK_SUCCESS) {
      fputs("An error occured while creating the framebuffers.", stderr);
      exit(EXIT_FAILURE);
    }
  }

  // Allocate the command buffers.
  VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
  commandBufferAllocateInfo.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocateInfo.commandPool = commandPool;
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandBufferCount = imageCount;

  commandBuffers =
      (VkCommandBuffer *)malloc(imageCount * sizeof(VkCommandBuffer));

  if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo,
                               commandBuffers) != VK_SUCCESS) {
    fputs("An error occured while creating the command buffers.", stderr);
    exit(EXIT_FAILURE);
  }

  // Create semaphores for rendering.
  VkSemaphoreCreateInfo semaphoreCreateInfo = {};
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  if (vkCreateSemaphore(device, &semaphoreCreateInfo, NULL,
                        &imageAvailableSemaphore) != VK_SUCCESS ||
      vkCreateSemaphore(device, &semaphoreCreateInfo, NULL,
                        &renderFinishedSemaphore) != VK_SUCCESS) {
    fputs("An error occured while creating the semaphores.", stderr);
  }
}

void flapRendererQuit() {
  vkDeviceWaitIdle(device);

  vkDestroySemaphore(device, renderFinishedSemaphore, NULL);
  vkDestroySemaphore(device, imageAvailableSemaphore, NULL);

  vkFreeCommandBuffers(device, commandPool, imageCount, commandBuffers);
  free(commandBuffers);

  vkDestroyCommandPool(device, commandPool, NULL);

  for (uint32_t i = 0; i < imageCount; i++) {
    vkDestroyFramebuffer(device, framebuffers[i], NULL);
    vkDestroyImageView(device, swapchainImageViews[i], NULL);
  }

  free(framebuffers);
  free(swapchainImageViews);
  free(swapchainImages);

  vkDestroyRenderPass(device, renderPass, NULL);

  vkDestroySwapchainKHR(device, swapchain, NULL);

  vkDestroyDevice(device, NULL);

  vkDestroyInstance(instance, NULL);
}

void flapRendererRender() {
  uint32_t imageIndex;
  vkAcquireNextImageKHR(device, swapchain, UINT32_MAX, imageAvailableSemaphore,
                        NULL, &imageIndex);

  VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
  submitInfo.pWaitDstStageMask = &waitStage;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

  if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, NULL) != VK_SUCCESS) {
    fputs("Error submitting command buffers.", stderr);
    exit(EXIT_FAILURE);
  }

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swapchain;
  presentInfo.pImageIndices = &imageIndex;

  vkQueuePresentKHR(presentQueue, &presentInfo);
  vkQueueWaitIdle(presentQueue);
}

VkDevice flapRendererGetDevice() { return device; }

const VkExtent2D flapRendererGetExtent() { return imageExtent; }

const VkRenderPass flapRendererGetRenderPass() { return renderPass; }

void flapRendererSetPipeline(VkPipeline nextPipeline,
                             VkPipelineLayout nextPipelineLayout) {
  pipeline = nextPipeline;
  pipelineLayout = nextPipelineLayout;
}

void flapRendererSetVertexBuffer(VkBuffer buffer) { vertexBuffer = buffer; }

void flapRendererSetIndexBuffer(VkBuffer buffer) { indexBuffer = buffer; }

void flapRendererRecordCommandBuffers() {
  if (pipeline == VK_NULL_HANDLE) {
    fputs("Recording command buffers without a graphics pipeline.", stderr);
    exit(EXIT_FAILURE);
  } else if (vertexBuffer == VK_NULL_HANDLE) {
    fputs("Recording command buffers without a vertex buffer.", stderr);
    exit(EXIT_FAILURE);
  } else if (indexBuffer == VK_NULL_HANDLE) {
    fputs("Recording command buffers without an index buffer.", stderr);
    exit(EXIT_FAILURE);
  }

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  for (uint32_t i = 0; i < imageCount; ++i) {
    vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

    const VkClearValue clearColor = {{{0.53f, 0.81f, 0.92f, 1.f}}};

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = framebuffers[i];
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = imageExtent;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, &offset);

    vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0,
                         VK_INDEX_TYPE_UINT16);

    // Draw player
    vkCmdPushConstants(commandBuffers[i], pipelineLayout,
                       VK_SHADER_STAGE_FRAGMENT_BIT, 0, 3 * sizeof(float),
                       FLAP_BIRD_COLOR);
    vkCmdDrawIndexed(commandBuffers[i], 6, 1, 0, 0, 0);

    // Draw pipes
    vkCmdPushConstants(commandBuffers[i], pipelineLayout,
                       VK_SHADER_STAGE_FRAGMENT_BIT, 0, 3 * sizeof(float),
                       FLAP_PIPE_COLOR);
    vkCmdDrawIndexed(commandBuffers[i], (1 + FLAP_NUM_PIPES * 2) * 6, 1, 6, 0,
                     0);

    vkCmdEndRenderPass(commandBuffers[i]);

    vkEndCommandBuffer(commandBuffers[i]);
  }
}

void flapRendererCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                              VkMemoryPropertyFlags memoryProperties,
                              VkBuffer *buffer, VkDeviceMemory *bufferMemory) {

  VkBufferCreateInfo bufferCreateInfo = {};
  bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCreateInfo.size = size;
  bufferCreateInfo.usage = usage;
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  vkCreateBuffer(device, &bufferCreateInfo, NULL, buffer);

  VkMemoryRequirements memoryRequirements;
  vkGetBufferMemoryRequirements(device, *buffer, &memoryRequirements);

  uint32_t bestMemory = 0;
  for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount;
       i++) {
    VkMemoryType memType = physicalDeviceMemoryProperties.memoryTypes[i];
    if ((memoryProperties & (1 << i)) &&
        (memType.propertyFlags & memoryProperties)) {
      bestMemory = i;
    }
  }

  VkMemoryAllocateInfo allocateInfo = {};
  allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocateInfo.allocationSize = size;
  allocateInfo.memoryTypeIndex = bestMemory;

  vkAllocateMemory(device, &allocateInfo, NULL, bufferMemory);

  vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

void flapRendererBufferData(VkDeviceMemory bufferMemory, VkDeviceSize size,
                            void *data) {
  uint8_t *destMemory;
  if (vkMapMemory(device, bufferMemory, 0, size, 0, (void *)&destMemory) !=
      VK_SUCCESS) {
    fputs("An error occured while mapping device memory.", stderr);
    exit(EXIT_FAILURE);
  }
  memcpy(destMemory, data, (size_t)size);
  vkUnmapMemory(device, bufferMemory);
}