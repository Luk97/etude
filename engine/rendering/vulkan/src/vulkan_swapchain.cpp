#include "vulkan_swapchain.h"

#include "vulkan_check.h"
#include "vulkan_semaphore.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

namespace etude::vulkan {

    namespace {

        constexpr std::uint64_t noTimeout = std::numeric_limits<std::uint64_t>::max();

        /// @brief Returns the size of the images. Most window systems dictate it through currentExtent, the special
        /// value 0xFFFFFFFF leaves the choice to the swapchain within the allowed range.
        VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, Size windowSize) {
            if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
                return capabilities.currentExtent;
            }
            return {
                std::clamp(
                    static_cast<std::uint32_t>(windowSize.width), capabilities.minImageExtent.width,
                    capabilities.maxImageExtent.width
                ),
                std::clamp(
                    static_cast<std::uint32_t>(windowSize.height), capabilities.minImageExtent.height,
                    capabilities.maxImageExtent.height
                )
            };
        }

        /// @brief Asks for one image more than the minimum, so that the renderer does not have to wait for the display
        /// to release one. A maximum of 0 means there is no upper limit.
        std::uint32_t chooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities) {
            const std::uint32_t count = capabilities.minImageCount + 1;
            return capabilities.maxImageCount == 0 ? count : std::min(count, capabilities.maxImageCount);
        }
    }

    VkSurfaceFormatKHR chooseSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
        std::uint32_t count = 0;
        check(
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr),
            "vkGetPhysicalDeviceSurfaceFormatsKHR"
        );
        std::vector<VkSurfaceFormatKHR> formats(count);
        check(
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, formats.data()),
            "vkGetPhysicalDeviceSurfaceFormatsKHR"
        );

        const auto srgb = std::ranges::find_if(formats, [](const VkSurfaceFormatKHR& format) {
            return format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        });
        return srgb != formats.end() ? *srgb : formats.front();
    }

    Swapchain createSwapchain(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkSurfaceKHR surface,
        VkSurfaceFormatKHR format,
        Size windowSize
    ) {
        VkSurfaceCapabilitiesKHR capabilities{};
        check(
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities),
            "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"
        );
        const VkExtent2D extent = chooseExtent(capabilities, windowSize);

        // FIFO waits for the display refresh and is the only present mode that every Vulkan device supports.
        const VkSwapchainCreateInfoKHR info{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = chooseImageCount(capabilities),
            .imageFormat = format.format,
            .imageColorSpace = format.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_FIFO_KHR,
            .clipped = VK_TRUE,
        };

        VkSwapchainKHR handle = nullptr;
        check(vkCreateSwapchainKHR(device, &info, nullptr, &handle), "vkCreateSwapchainKHR");

        Swapchain swapchain{
            .handle = SwapchainHandle(handle, {device}),
            .size = {
                static_cast<int>(extent.width),
                static_cast<int>(extent.height),
            },
        };

        std::uint32_t count = 0;
        check(vkGetSwapchainImagesKHR(device, handle, &count, nullptr), "vkGetSwapchainImagesKHR");
        swapchain.images.resize(count);
        check(vkGetSwapchainImagesKHR(device, handle, &count, swapchain.images.data()), "vkGetSwapchainImagesKHR");

        for (VkImage image : swapchain.images) {
            swapchain.views.push_back(createImageView(device, image, format.format));
            swapchain.renderFinished.push_back(createSemaphore(device));
        }
        return swapchain;
    }

    std::optional<std::uint32_t> acquireImage(VkDevice device, const Swapchain& swapchain, VkSemaphore imageAvailable) {
        std::uint32_t index = 0;
        const VkResult result =
            vkAcquireNextImageKHR(device, swapchain.handle.get(), noTimeout, imageAvailable, nullptr, &index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            return std::nullopt;
        }

        // A suboptimal swapchain can still show the image, presenting reports it again and triggers the rebuild.
        if (result != VK_SUBOPTIMAL_KHR) {
            check(result, "vkAcquireNextImageKHR");
        }
        return index;
    }

    bool presentImage(VkQueue queue, const Swapchain& swapchain, std::uint32_t imageIndex) {
        const VkSemaphore wait = swapchain.renderFinished[imageIndex].get();
        const VkSwapchainKHR handle = swapchain.handle.get();
        const VkPresentInfoKHR info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &wait,
            .swapchainCount = 1,
            .pSwapchains = &handle,
            .pImageIndices = &imageIndex,
        };
        const VkResult result = vkQueuePresentKHR(queue, &info);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            return false;
        }
        check(result, "vkQueuePresentKHR");
        return true;
    }
}
