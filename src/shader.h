#ifndef FLAP_SHADER_H_
#define FLAP_SHADER_H_
#include <vulkan/vulkan.h>

#include "device.h"

/**
 * Contains a shader module.
 */
typedef VkPipelineShaderStageCreateInfo Shader;

/**
 * Compile a shader module from SPIR-V source.
 */
void shader_create(Device *device, const char *file_name,
                   VkShaderStageFlags stage, Shader *shader);

/**
 * Destroy a shader module.
 */
void shader_destroy(Device *device, Shader *shader);

#endif // FLAP_SHADER_H_
