#include "rect.h"

#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include "flap.h"

#include "vk/pipeline.h"
#include "vk/renderer.h"

#include "vk/shaders.h"

static flapPipeline pipeline;

static VkBuffer vertexBuffer = VK_NULL_HANDLE;
static VkBuffer indexBuffer = VK_NULL_HANDLE;

static VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
static VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

static const VkDeviceSize verticesSize =
    (1 + FLAP_NUM_PIPES * 2) * sizeof(flapRect);
static const VkDeviceSize indicesSize =
    (1 + FLAP_NUM_PIPES * 2) * 6 * sizeof(uint16_t);

void flapRectInit() {
  pipeline = flapPipelineCreate(rect_vert_spv_len, rect_vert_spv,
                                rect_frag_spv_len, rect_frag_spv);

  flapRendererCreateBuffer(verticesSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           &vertexBuffer, &vertexBufferMemory);

  flapRendererCreateBuffer(indicesSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           &indexBuffer, &indexBufferMemory);

  flapRendererSetPipeline(pipeline.pipeline, pipeline.pipelineLayout);
  flapRendererSetVertexBuffer(vertexBuffer);
  flapRendererSetIndexBuffer(indexBuffer);

  flapRendererRecordCommandBuffers();
}

void flapRectQuit() {
  VkDevice device = flapRendererGetDevice();

  vkDestroyBuffer(device, vertexBuffer, NULL);
  vkFreeMemory(device, vertexBufferMemory, NULL);

  vkDestroyBuffer(device, indexBuffer, NULL);
  vkFreeMemory(device, indexBufferMemory, NULL);

  flapPipelineDestroy(pipeline);
}

void flapRectDraw() {
  flapRendererBufferData(vertexBufferMemory, verticesSize,
                         flapRectGetVertices());
  flapRendererBufferData(indexBufferMemory, indicesSize, flapRectGetIndices());
}
