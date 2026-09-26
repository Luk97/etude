#pragma once

#include "vulkan_handles.h"

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    /// @brief Creates a semaphore, which orders work between operations on the GPU.
    Semaphore createSemaphore(VkDevice device);

    /// @brief Creates a fence, through which the CPU waits for work on the GPU.
    Fence createFence(VkDevice device, VkFenceCreateFlags flags);
}
