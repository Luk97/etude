#include "vulkan_image.h"

#include "vulkan_check.h"

namespace etude::vulkan {

    namespace {

        /// @brief The single color layer of a swapchain image.
        constexpr VkImageSubresourceRange colorLayer{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1,
        };

        /// @brief Records a pipeline barrier with a single image barrier.
        void recordImageBarrier(VkCommandBuffer commands, const VkImageMemoryBarrier2& barrier) {
            const VkDependencyInfo dependency{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &barrier,
            };
            vkCmdPipelineBarrier2(commands, &dependency);
        }
    }

    ImageView createImageView(VkDevice device, VkImage image, VkFormat format) {
        const VkImageViewCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .subresourceRange = colorLayer,
        };

        VkImageView view = nullptr;
        check(vkCreateImageView(device, &info, nullptr, &view), "vkCreateImageView");
        return ImageView(view, {device});
    }

    void transitionToColorTarget(VkCommandBuffer commands, VkImage image) {
        const VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = image,
            .subresourceRange = colorLayer,
        };
        recordImageBarrier(commands, barrier);
    }

    void transitionToPresent(VkCommandBuffer commands, VkImage image) {
        const VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .image = image,
            .subresourceRange = colorLayer,
        };
        recordImageBarrier(commands, barrier);
    }
}
