#include "vulkan_swapchain.h"

#include "vulkan_check.h"
#include "vulkan_sync.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

namespace etude {

    namespace {

        /// @brief Prefers 8-bit sRGB, so that the display applies the gamma curve to the colors the shaders write.
        /// Falls back to the first format the surface offers.
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
                return format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                       format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            });
            return srgb != formats.end() ? *srgb : formats.front();
        }

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

        ImageView createImageView(VkDevice device, VkImage image, VkFormat format) {
            const VkImageViewCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = format,
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .levelCount = 1,
                    .layerCount = 1,
                },
            };

            VkImageView view = nullptr;
            check(vkCreateImageView(device, &info, nullptr, &view), "vkCreateImageView");
            return ImageView(view, ImageViewDeleter{device});
        }
    }

    VulkanSwapchain createSwapchain(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkSurfaceKHR surface,
        Size windowSize
    ) {
        VkSurfaceCapabilitiesKHR capabilities{};
        check(
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities),
            "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"
        );
        const VkSurfaceFormatKHR format = chooseSurfaceFormat(physicalDevice, surface);
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

        VulkanSwapchain swapchain{
            .handle = SwapchainHandle(handle, SwapchainDeleter{device}),
            .format = format.format,
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
}
