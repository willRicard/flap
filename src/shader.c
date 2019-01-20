#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "assets.h"
#include "error.h"
#include "renderer.h"

VkShaderModule shader_create(const char *source_file) {
  VkShaderModuleCreateInfo module_info = {0};
  module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  module_info.pNext = NULL;
  module_info.flags = 0;

  uint32_t *shader_code = NULL;
  shader_code =
      (uint32_t *)assets_read_file(source_file, &module_info.codeSize);
  if (shader_code == NULL) {
    fail_with_error("Failed reading shader code.");
  }
  module_info.pCode = (const uint32_t *)shader_code;

  VkShaderModule module = VK_NULL_HANDLE;
  if (vkCreateShaderModule(renderer_get_device(), &module_info, NULL,
                           &module) != VK_SUCCESS) {
    fail_with_error("Error while creating the vertex shader module.");
  }

  free(shader_code);

  return module;
}

void shader_destroy(VkShaderModule module) {
  vkDestroyShaderModule(renderer_get_device(), module, NULL);
}
