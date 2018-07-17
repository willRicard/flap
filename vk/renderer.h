#ifndef FLAP_RENDERER_H_
#define FLAP_RENDERER_H_

#include <vulkan/vulkan.h>

void flapRendererInit();

void flapRendererQuit();

void flapRendererRender();

VkDevice flapRendererGetDevice();

const VkExtent2D flapRendererGetExtent();

const VkRenderPass flapRendererGetRenderPass();

void flapRendererSetVertexBuffer(VkBuffer buffer);

void flapRendererSetIndexBuffer(VkBuffer buffer);

void flapRendererSetPipeline(VkPipeline pipeline, VkPipelineLayout pipelineLayout);

void flapRendererRecordCommandBuffers();

void flapRendererCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                              VkMemoryPropertyFlags memoryProperties,
                              VkBuffer *buffer, VkDeviceMemory *bufferMemory);

void flapRendererBufferData(VkDeviceMemory bufferMemory, VkDeviceSize size,
                            const void *data);

#endif // FLAP_RENDERER_H_
