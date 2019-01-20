#ifndef FLAP_SHADER_H_
#define FLAP_SHADER_H_
#include <vulkan/vulkan.h>

/**
 * Compile a shader module from SPIR-V source.
 */
VkShaderModule shader_create(const char *file_name);

/**
 * Destroy a shader module.
 */
void shader_destroy(VkShaderModule module);

#endif // FLAP_SHADER_H_
