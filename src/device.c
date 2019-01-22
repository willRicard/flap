#include "device.h"

#include "error.h"
#include "window.h"

void device_create(VkInstance instance, VkSurfaceKHR surface, Device *dev) {
  uint32_t device_count = 0;
  error_check(vkEnumeratePhysicalDevices(instance, &device_count, NULL),
              "vkEnumeratePhysicalDevices");

  VkPhysicalDevice devices[2];

  error_check(vkEnumeratePhysicalDevices(instance, &device_count, devices),
              "vkEnumeratePhysicalDevices");

  VkPhysicalDeviceProperties physical_device_properties;
  for (uint32_t i = 0; i < device_count; i++) {
    vkGetPhysicalDeviceProperties(devices[i], &physical_device_properties);
    if (physical_device_properties.deviceType ==
        VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      dev->physical_device = devices[i];
      break;
    } else if (physical_device_properties.deviceType ==
               VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
      dev->physical_device = devices[i];
    }
  }
  if (dev->physical_device == VK_NULL_HANDLE) {
    window_fail_with_error("No suitable device was found.");
  }

  // Query memory properties for later allocations
  vkGetPhysicalDeviceMemoryProperties(dev->physical_device,
                                      &dev->physical_device_memory_properties);

  // Pick the best graphics & present queues
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(dev->physical_device,
                                           &queue_family_count, NULL);

  VkQueueFamilyProperties queue_family_properties[2];

  vkGetPhysicalDeviceQueueFamilyProperties(
      dev->physical_device, &queue_family_count, queue_family_properties);

  uint32_t graphics_queue_id = 0;
  uint32_t present_queue_id = 0;

  for (uint32_t i = 0; i < queue_family_count; i++) {
    VkQueueFamilyProperties queue_family = queue_family_properties[i];
    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphics_queue_id = i;
      break;
    } else if (i == queue_family_count - 1) {
      window_fail_with_error("No graphics queue was found.");
    }
  }
  for (uint32_t i = 0; i < queue_family_count; i++) {
    VkBool32 present_supported;
    vkGetPhysicalDeviceSurfaceSupportKHR(dev->physical_device, i, surface,
                                         &present_supported);
    if (present_supported) {
      present_queue_id = i;
      break;
    } else if (i == queue_family_count - 1) {
      window_fail_with_error("No present queue was found.");
    }
  }

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
  vkGetPhysicalDeviceFeatures(dev->physical_device, &features);

  static const char *deviceExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  VkDeviceCreateInfo device_info = {0};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = NULL;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = queue_infos;
  device_info.enabledExtensionCount = 1;
  device_info.ppEnabledExtensionNames = &deviceExtension;
  device_info.pEnabledFeatures = &features;

  vkCreateDevice(dev->physical_device, &device_info, NULL, &dev->device);

  vkGetDeviceQueue(dev->device, graphics_queue_id, 0, &dev->graphics_queue);
  vkGetDeviceQueue(dev->device, present_queue_id, 0, &dev->present_queue);

  // Create the command pool
  VkCommandPoolCreateInfo pool_info = {0};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = graphics_queue_id;

  error_check(
      vkCreateCommandPool(dev->device, &pool_info, NULL, &dev->command_pool),
      "vkCreateCommandPool");
}

void device_destroy(Device *dev) {
  vkDestroyCommandPool(dev->device, dev->command_pool, NULL);
  vkDestroyDevice(dev->device, NULL);
}

void device_begin_command_buffer(Device *device, VkCommandBuffer *cmd_buf) {
  VkCommandBufferAllocateInfo command_buffer_info = {0};
  command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_info.commandPool = device->command_pool;
  command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  command_buffer_info.commandBufferCount = 1;

  error_check(
      vkAllocateCommandBuffers(device->device, &command_buffer_info, cmd_buf),
      "vkAllocateCommandBuffers");

  VkCommandBufferBeginInfo begin_info = {0};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(*cmd_buf, &begin_info);
}

void device_end_command_buffer(Device *device, VkCommandBuffer *cmd_buf) {
  vkEndCommandBuffer(*cmd_buf);

  VkSubmitInfo submit_info = {0};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = cmd_buf;

  error_check(
      vkQueueSubmit(device->graphics_queue, 1, &submit_info, VK_NULL_HANDLE),
      "vkQueueSubmit");
  vkQueueWaitIdle(device->graphics_queue);

  vkFreeCommandBuffers(device->device, device->command_pool, 1, cmd_buf);
}

uint32_t device_find_memory_type(Device *device,
                                 VkMemoryRequirements mem_requirements,
                                 VkMemoryPropertyFlags mem_properties) {
  uint32_t best_memory = 0;
  VkBool32 memory_found = VK_FALSE;
  for (uint32_t i = 0;
       i < device->physical_device_memory_properties.memoryTypeCount; i++) {
    VkMemoryType memory_type =
        device->physical_device_memory_properties.memoryTypes[i];
    if ((mem_requirements.memoryTypeBits & (1 << i)) &&
        (memory_type.propertyFlags & mem_properties)) {
      best_memory = i;
      memory_found = VK_TRUE;
      break;
    }
  }
  if (memory_found == VK_FALSE) {
    window_fail_with_error(
        "Allocation failed: no suitable memory type could be found!");
  }
  return best_memory;
}
