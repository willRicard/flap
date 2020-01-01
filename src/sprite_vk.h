#pragma once
#include "sprite.h"
#include <sulfur/device.h>
#include <vulkan/vulkan.h>

/**
 * Load shaders and resources.
 */
void sprite_vk_init(SulfurDevice *dev);

/**
 * Free shaders and resources.
 */
void sprite_vk_quit(SulfurDevice *device);

/**
 * Build pipeline create info.
 */
void sprite_get_pipeline_create_info(
    VkGraphicsPipelineCreateInfo *pipeline_info);

VkPipelineLayout sprite_get_pipeline_layout(void);

/**
 * Get the descriptor set layout for the texture.
 */
VkDescriptorSetLayout sprite_get_descriptor_set_layout(void);

/**
 * Create a descriptor set for the texture.
 */
void sprite_create_descriptor(SulfurDevice *dev,
                              VkDescriptorSet descriptor_set);

/*
 * Record command buffers.
 */
void sprite_record_command_buffer(VkCommandBuffer cmd_buf);
