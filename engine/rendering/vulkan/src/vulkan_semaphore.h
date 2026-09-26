#pragma once

#include "vulkan_handles.h"

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using Semaphore = DeviceChild<VkSemaphore, vkDestroySemaphore>;

    /// @brief Creates a semaphore, which orders work between operations on the GPU.
    Semaphore createSemaphore(VkDevice device);
}
