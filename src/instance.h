#ifndef FLAP_INSTANCE_H
#define FLAP_INSTANCE_H
#include <vulkan/vulkan.h>

/**
 * Create a Vulkan instance.
 */
VkInstance instance_create();

/**
 * Destroy a Vulkan instance.
 */
void instance_destroy();

#endif // FLAP_INSTANCE_H
