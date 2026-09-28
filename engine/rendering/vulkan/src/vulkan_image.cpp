#include "vulkan_image.h"

#include "vulkan_check.h"

#include <cstdint>

namespace etude::vulkan {

    namespace {

        /// @brief The single color layer of the images that the renderer uses.
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

    GpuImage createImage(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        Size size,
        VkFormat format,
        VkImageUsageFlags usage
    ) {
        const VkExtent3D extent{
            .width = static_cast<std::uint32_t>(size.width),
            .height = static_cast<std::uint32_t>(size.height),
            .depth = 1,
        };
        const VkImageCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = extent,
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VkImage handle = nullptr;
        check(vkCreateImage(device, &info, nullptr, &handle), "vkCreateImage");

        VkMemoryRequirements requirements{};
        vkGetImageMemoryRequirements(device, handle, &requirements);
        GpuImage image{
            .allocation = allocateMemory(physicalDevice, device, requirements, MemoryAccess::GpuOnly, 0),
            .handle = ImageHandle(handle, {device}),
        };
        check(
            vkBindImageMemory(device, handle, image.allocation.memory.get(), image.allocation.offset),
            "vkBindImageMemory"
        );
        image.view = createImageView(device, handle, format);
        return image;
    }

    void transitionToColorTarget(VkCommandBuffer commands, VkImage image) {
        const VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = image,
            .subresourceRange = colorLayer,
        };
        recordImageBarrier(commands, barrier);
    }

    void transitionToBlitSource(VkCommandBuffer commands, VkImage image) {
        const VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .image = image,
            .subresourceRange = colorLayer,
        };
        recordImageBarrier(commands, barrier);
    }

    void transitionToBlitTarget(VkCommandBuffer commands, VkImage image) {
        const VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .image = image,
            .subresourceRange = colorLayer,
        };
        recordImageBarrier(commands, barrier);
    }

    void transitionToPresent(VkCommandBuffer commands, VkImage image) {
        const VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .image = image,
            .subresourceRange = colorLayer,
        };
        recordImageBarrier(commands, barrier);
    }

    void transitionToTransferTarget(VkCommandBuffer commands, VkImage image) {
        const VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .image = image,
            .subresourceRange = colorLayer,
        };
        recordImageBarrier(commands, barrier);
    }

    void transitionToShaderRead(VkCommandBuffer commands, VkImage image) {
        const VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .image = image,
            .subresourceRange = colorLayer,
        };
        recordImageBarrier(commands, barrier);
    }

    void blitImage(VkCommandBuffer commands, VkImage source, VkImage target, Size size) {
        const VkImageSubresourceLayers layer{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .layerCount = 1,
        };
        const VkOffset3D corner{size.width, size.height, 1};
        const VkImageBlit region{
            .srcSubresource = layer,
            .srcOffsets = {{}, corner},
            .dstSubresource = layer,
            .dstOffsets = {{}, corner},
        };
        vkCmdBlitImage(
            commands, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
            &region, VK_FILTER_NEAREST
        );
    }
}
