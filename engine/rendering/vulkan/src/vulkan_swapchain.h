#pragma once

#include "vulkan_handles.h"
#include "vulkan_image.h"
#include "vulkan_semaphore.h"

#include <etude/math/size.h>

#include <cstdint>
#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using SwapchainHandle = DeviceChild<VkSwapchainKHR, vkDestroySwapchainKHR>;

    /// @brief A swapchain with its images and one view per image, ready to be rendered into.
    /// The members are destroyed in reverse order, so the views go before the swapchain that owns their images.
    struct Swapchain {
        SwapchainHandle handle;
        std::vector<VkImage> images;
        std::vector<ImageView> views;

        /// @brief Signaled when an image is rendered and may be presented. It belongs to the image and not to the frame
        /// in flight, because presenting may still wait on it when that frame comes around again.
        std::vector<Semaphore> renderFinished;

        Size size;
    };

    /// @brief Prefers 8-bit sRGB, so that the display applies the gamma curve to the colors the shaders write.
    /// Falls back to the first format the surface offers.
    VkSurfaceFormatKHR chooseSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    /// @brief Creates a swapchain for the surface with images of the given format that fit a window of the given size.
    Swapchain createSwapchain(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkSurfaceKHR surface,
        VkSurfaceFormatKHR format,
        Size windowSize
    );

    /// @brief Returns the index of the next image to render into, or nothing if the swapchain no longer fits the
    /// surface and has to be rebuilt. The semaphore is signaled once the display has really released the image.
    std::optional<std::uint32_t> acquireImage(VkDevice device, const Swapchain& swapchain, VkSemaphore imageAvailable);

    /// @brief Shows the image in the window once its renderFinished semaphore is signaled. Returns false if the
    /// swapchain no longer fits the surface and has to be rebuilt.
    bool presentImage(VkQueue queue, const Swapchain& swapchain, std::uint32_t imageIndex);
}
