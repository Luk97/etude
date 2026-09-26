#pragma once

#include "vulkan_handles.h"

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using Memory = DeviceChild<VkDeviceMemory, vkFreeMemory>;

    /// @brief How the CPU reaches the memory of a resource.
    enum class MemoryAccess {
        /// @brief Only the GPU reads and writes it. The fastest choice for textures and data that rarely changes.
        GpuOnly,
        /// @brief The CPU writes it through a pointer that stays mapped, and every write reaches the GPU without a
        /// flush. For uploads and for data that changes every frame.
        CpuWrite,
    };

    /// @brief The memory of one resource. For now every resource gets an allocation of its own. Later the allocator
    /// can hand out ranges of large blocks, which is why resources bind their memory at an offset.
    struct Allocation {
        Memory memory;
        VkDeviceSize offset = 0;
        void* mapped = nullptr;
    };

    /// @brief Allocates memory that fits the requirements of a buffer or an image. Buffers whose address shaders read
    /// need the flag VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT.
    Allocation allocateMemory(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        const VkMemoryRequirements& requirements,
        MemoryAccess access,
        VkMemoryAllocateFlags flags
    );
}
