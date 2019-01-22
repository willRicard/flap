#include "error.h"

#include "window.h"

void error_check(VkResult result, const char *message) {
  if (result != VK_SUCCESS) {
    window_fail_with_error(message);
  }
}
