#pragma once

#include "vulkan_handles.h"
#include "vulkan_memory.h"

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using BufferHandle = DeviceChild<VkBuffer, vkDestroyBuffer>;

    /// @brief A buffer with its memory. The members are destroyed in reverse order, so the buffer goes before its
    /// memory.
    struct Buffer {
        Allocation allocation;
        BufferHandle handle;

        /// @brief Where shaders find the buffer, like a pointer. Only set for buffers with the usage
        /// VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT.
        VkDeviceAddress address = 0;

        VkDeviceSize size = 0;
    };

    /// @brief Creates a buffer of the given size and usage in memory with the given access.
    Buffer createBuffer(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        MemoryAccess access
    );
}
