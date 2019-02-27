#ifndef FLAP_PIPELINE_H
#define FLAP_PIPELINE_H
#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"

/**
 * Load the pipeline cache from `pipeline_cache.bin`.
 */
VkPipelineCache pipeline_cache_create(Device *device);

/**
 * Write the pipeline cache to `pipeline_cache.bin` then destroy it.
 */
void pipeline_cache_destroy(Device *device, VkPipelineCache pipeline_cache);

/**
 * Default pipeline create info with convenient defaults.
 */
void pipeline_make_default_create_info(Swapchain *swapchain,
                                       VkGraphicsPipelineCreateInfo *info);

#endif // FLAP_PIPELINE_H
