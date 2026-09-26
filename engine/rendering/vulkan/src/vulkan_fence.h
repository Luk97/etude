#pragma once

#include "vulkan_handles.h"

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using Fence = DeviceChild<VkFence, vkDestroyFence>;

    /// @brief Creates a fence, through which the CPU waits for work on the GPU.
    Fence createFence(VkDevice device, VkFenceCreateFlags flags);

    /// @brief Blocks until the GPU has signaled the fence.
    void waitForFence(VkDevice device, VkFence fence);

    /// @brief Makes the fence unsignaled again, so that the next submit can signal it.
    void resetFence(VkDevice device, VkFence fence);
}
