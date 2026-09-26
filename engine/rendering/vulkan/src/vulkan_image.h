#pragma once

#include "vulkan_handles.h"

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using ImageView = DeviceChild<VkImageView, vkDestroyImageView>;

    /// @brief Creates a view onto the color layer of a 2D image in the given format.
    ImageView createImageView(VkDevice device, VkImage image, VkFormat format);

    /// @brief Records the transition of an image whose old content does not matter to a color target. The barrier
    /// waits in the stage in which the submit waits for the acquired image.
    void transitionToColorTarget(VkCommandBuffer commands, VkImage image);

    /// @brief Records the transition of a rendered color target to presenting. Presenting reads the image outside of
    /// the pipeline, the semaphore that the submit signals orders it after this barrier.
    void transitionToPresent(VkCommandBuffer commands, VkImage image);
}
