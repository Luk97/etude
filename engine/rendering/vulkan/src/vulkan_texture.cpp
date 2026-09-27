#include "vulkan_texture.h"

#include "vulkan_buffer.h"
#include "vulkan_check.h"
#include "vulkan_command_buffer.h"

#include <cstdint>
#include <cstring>

namespace etude::vulkan {

    namespace {

        /// @brief Images store their color in sRGB. Sampling a texture in this format returns linear colors, and the
        /// sRGB swapchain converts them back when it writes, so the colors reach the screen as they were drawn.
        constexpr VkFormat textureFormat = VK_FORMAT_R8G8B8A8_SRGB;

        static_assert(sizeof(Pixel) == 4, "The upload copies the pixels byte for byte into the RGBA texture.");

        /// @brief The size of the image as a Vulkan extent, one texel deep.
        VkExtent3D extentOf(const Image& image) {
            return {
                .width = static_cast<std::uint32_t>(image.width),
                .height = static_cast<std::uint32_t>(image.height),
                .depth = 1,
            };
        }

        /// @brief Copies the pixels of the image into the target and leaves it for fragment shaders. The CPU cannot
        /// write into an optimally tiled image, whose texels lie in an order that only the GPU knows. So the pixels
        /// go into a staging buffer first, and the GPU copies them from there.
        void copyPixels(
            VkPhysicalDevice physicalDevice,
            VkDevice device,
            VkQueue queue,
            VkCommandBuffer commands,
            const Image& image,
            VkImage target
        ) {
            const VkDeviceSize size = image.pixels.size() * sizeof(Pixel);
            const Buffer staging =
                createBuffer(physicalDevice, device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryAccess::CpuWrite);
            std::memcpy(staging.allocation.mapped, image.pixels.data(), size);

            const VkImageSubresourceLayers colorLayer{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .layerCount = 1,
            };
            const VkBufferImageCopy region{
                .imageSubresource = colorLayer,
                .imageExtent = extentOf(image),
            };

            beginCommands(commands);
            transitionToTransferTarget(commands, target);
            vkCmdCopyBufferToImage(
                commands, staging.handle.get(), target, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region
            );
            transitionToShaderRead(commands, target);
            endCommands(commands);

            // The staging buffer is destroyed when the function returns, so the copy has to be done by then.
            submitAndWait(queue, commands);
        }
    }

    Texture createTexture(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkQueue queue,
        VkCommandBuffer commands,
        const Image& image
    ) {
        const VkImageCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = textureFormat,
            .extent = extentOf(image),
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VkImage handle = nullptr;
        check(vkCreateImage(device, &info, nullptr, &handle), "vkCreateImage");

        VkMemoryRequirements requirements{};
        vkGetImageMemoryRequirements(device, handle, &requirements);
        Texture texture{
            .allocation = allocateMemory(physicalDevice, device, requirements, MemoryAccess::GpuOnly, 0),
            .image = ImageHandle(handle, {device}),
        };
        check(
            vkBindImageMemory(device, handle, texture.allocation.memory.get(), texture.allocation.offset),
            "vkBindImageMemory"
        );
        texture.view = createImageView(device, handle, textureFormat);

        copyPixels(physicalDevice, device, queue, commands, image, handle);
        return texture;
    }
}
