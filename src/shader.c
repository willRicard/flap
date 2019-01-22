#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "assets.h"
#include "error.h"
#include "window.h"

void shader_create(Device *dev, const char *source_file,
                   VkShaderStageFlags stage, Shader *shader) {
  VkShaderModuleCreateInfo module_info = {0};
  module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  module_info.pNext = NULL;
  module_info.flags = 0;

  uint32_t *shader_code = NULL;
  shader_code =
      (uint32_t *)assets_read_file(source_file, &module_info.codeSize);
  if (shader_code == NULL) {
    window_fail_with_error("Failed reading shader code.");
  }
  module_info.pCode = (const uint32_t *)shader_code;

  if (vkCreateShaderModule(dev->device, &module_info, NULL, &shader->module) !=
      VK_SUCCESS) {
    window_fail_with_error("Error while creating the vertex shader module.");
  }

  free(shader_code);

  shader->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader->pNext = NULL;
  shader->flags = 0;
  shader->stage = stage;
  shader->pName = "main";
  shader->module = shader->module;
  shader->pSpecializationInfo = NULL;
}

void shader_destroy(Device *dev, Shader *shader) {
  vkDestroyShaderModule(dev->device, shader->module, NULL);
}
