#include "vulkan_texture.h"

#include "vulkan_buffer.h"
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

        /// @brief Copies the pixels of the image into the target and leaves it ready for fragment shaders. The CPU
        /// cannot write into an optimally tiled image, whose texels lie in an order that only the GPU knows. So the
        /// pixels go into a staging buffer first, and the GPU copies them from there.
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
        Texture texture = createImage(
            physicalDevice, device, {image.width, image.height}, textureFormat,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        );
        copyPixels(physicalDevice, device, queue, commands, image, texture.handle.get());
        return texture;
    }
}
