#ifndef FLAP_ERROR_H
#define FLAP_ERROR_H
#include <vulkan/vulkan.h>

/**
 * If `result` is not VK_SUCCESS, display an error message and exit the program.
 */
void error_check(VkResult result, const char *message);

#endif // FLAP_ERROR_H
