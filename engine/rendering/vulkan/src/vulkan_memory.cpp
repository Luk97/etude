#include "vulkan_memory.h"

#include "vulkan_check.h"

#include <etude/core/log.h>

#include <cstdint>
#include <cstdlib>

namespace etude::vulkan {

    namespace {

        std::uint32_t findMemoryType(
            VkPhysicalDevice physicalDevice,
            std::uint32_t allowedTypes,
            VkMemoryPropertyFlags properties
        ) {
            VkPhysicalDeviceMemoryProperties memory{};
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memory);
            for (std::uint32_t type = 0; type < memory.memoryTypeCount; ++type) {
                const bool allowed = (allowedTypes & (1u << type)) != 0;
                if (allowed && (memory.memoryTypes[type].propertyFlags & properties) == properties) {
                    return type;
                }
            }

            logFatal("No memory type has the properties {:#x}.", properties);
            std::abort();
        }
    }

    Allocation allocateMemory(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        const VkMemoryRequirements& requirements,
        MemoryAccess access,
        VkMemoryAllocateFlags flags
    ) {
        const VkMemoryPropertyFlags properties =
            access == MemoryAccess::GpuOnly
                ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        const VkMemoryAllocateFlagsInfo flagsInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
            .flags = flags,
        };
        const VkMemoryAllocateInfo info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = &flagsInfo,
            .allocationSize = requirements.size,
            .memoryTypeIndex = findMemoryType(physicalDevice, requirements.memoryTypeBits, properties),
        };

        VkDeviceMemory memory = nullptr;
        check(vkAllocateMemory(device, &info, nullptr, &memory), "vkAllocateMemory");
        Allocation allocation{
            .memory = Memory(memory, {device}),
        };

        // Mapped once for its whole lifetime, freeing the memory unmaps it again.
        if (access == MemoryAccess::CpuWrite) {
            check(vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, &allocation.mapped), "vkMapMemory");
        }

        return allocation;
    }
}
