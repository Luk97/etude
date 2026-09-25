#pragma once

#include "vulkan_handles.h"

#include <etude/core/size.h>

#include <vector>

#include <vulkan/vulkan.h>

namespace etude {

    /// @brief A swapchain with its images and one view per image, ready to be rendered into.
    /// The members are destroyed in reverse order, so the view go before the swapchain that own their images.
    struct VulkanSwapchain {
        SwapchainHandle handle;
        std::vector<VkImage> images;
        std::vector<ImageView> views;
        VkFormat format = VK_FORMAT_UNDEFINED;
        Size size;
    };

    /// @brief Creates a swapchain for the surface that fits a window of the given size.
    VulkanSwapchain createSwapchain(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkSurfaceKHR surface,
        Size windowSize
    );
}
