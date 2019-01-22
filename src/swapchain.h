#ifndef FLAP_SWAPCHAIN_H
#define FLAP_SWAPCHAIN_H
#include "device.h"
#include <vulkan/vulkan.h>

/**
 * Image swapchain.
 */
typedef struct {
  VkSwapchainKHR swapchain;

  VkSwapchainCreateInfoKHR info; /** Swapchain details */

  VkRenderPass render_pass; /** Write to color attachment. */

  uint32_t image_count; /** Number of multiple buffering images */
  VkImageView image_views[4];
  VkFramebuffer framebuffers[4];

  VkCommandBuffer command_buffers[4]; /** Command buffers run every frame */

  // Synchronization primitives
  VkSemaphore image_available_semaphore;
  VkSemaphore render_finished_semaphore;
} Swapchain;

/**
 * Create an image swapchain presenting to a window surface.
 */
void swapchain_create(Device *device, VkSurfaceKHR surface,
                      Swapchain *swapchain);

void swapchain_destroy(Device *device, Swapchain *swapchain);

/**
 * Recreate the swapchain, image views and framebuffers.
 */
void swapchain_resize(Device *device, VkSurfaceKHR surface,
                      Swapchain *swapchain);

/**
 * Destroy the swapchain, image views and framebuffers.
 */
void swapchain_cleanup(Device *device, Swapchain *swapchain);

/**
 * Submit the command buffers and present to the window surface.
 */
void swapchain_present(Device *device, VkSurfaceKHR surface,
                       Swapchain *swapchain);

#endif // FLAP_SWAPCHAIN_H
