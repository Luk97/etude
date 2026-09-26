#pragma once

#include "vulkan_handles.h"

#include <cstdint>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using CommandPool = DeviceChild<VkCommandPool, vkDestroyCommandPool>;

    /// @brief Creates the pool for the command buffers of a queue family. It allows resetting single buffers, because
    /// every frame records its own buffer anew.
    CommandPool createCommandPool(VkDevice device, std::uint32_t queueFamily);
}
