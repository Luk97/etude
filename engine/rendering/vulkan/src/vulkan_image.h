#pragma once

#include "vulkan_handles.h"
#include "vulkan_memory.h"

#include <etude/math/size.h>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using ImageHandle = DeviceChild<VkImage, vkDestroyImage>;
    using ImageView = DeviceChild<VkImageView, vkDestroyImageView>;

    /// @brief An image in GPU memory with a view onto its color layer, such as a texture or the scene image. The
    /// members are destroyed in reverse order: first the view, then the image, then its memory.
    struct GpuImage {
        Allocation allocation;
        ImageHandle handle;
        ImageView view;
    };

    /// @brief Creates a view onto the color layer of a 2D image in the given format.
    ImageView createImageView(VkDevice device, VkImage image, VkFormat format);

    /// @brief Creates a 2D image of the given size, format and usage in memory that only the GPU reaches, with a view
    /// onto its color layer. Its content stays undefined until the GPU writes it.
    GpuImage createImage(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        Size size,
        VkFormat format,
        VkImageUsageFlags usage
    );

    /// @brief Records the transition of the scene image to a color target. Its old content does not matter, but the
    /// blit of the previous frame may still read it, so the barrier waits for that blit.
    void transitionToColorTarget(VkCommandBuffer commands, VkImage image);

    /// @brief Records the transition of the drawn scene image to the source of a blit. The blit waits until the drawing
    /// has written the image.
    void transitionToBlitSource(VkCommandBuffer commands, VkImage image);

    /// @brief Records the transition of an acquired swapchain image to the target of a blit. Its old content does not
    /// matter, and the barrier waits in the stage in which the submit waits for the acquired image.
    void transitionToBlitTarget(VkCommandBuffer commands, VkImage image);

    /// @brief Records the transition of a swapchain image after the blit to presenting. Presenting reads the image
    /// outside of the pipeline, the semaphore that the submit signals orders it after this barrier.
    void transitionToPresent(VkCommandBuffer commands, VkImage image);

    /// @brief Records the transition of a new image to the target of a copy. Its old content does not matter, and no
    /// earlier work touches it, so the barrier waits for nothing.
    void transitionToTransferTarget(VkCommandBuffer commands, VkImage image);

    /// @brief Records the transition of a copied image to reading in fragment shaders. The barrier makes the shaders
    /// wait until the copy has written the image.
    void transitionToShaderRead(VkCommandBuffer commands, VkImage image);

    /// @brief Records a blit of the whole source image onto the whole target image, both of the given size. Unlike a
    /// copy, a blit can also scale, which the scene needs once its size differs from the window.
    void blitImage(VkCommandBuffer commands, VkImage source, VkImage target, Size size);
}
