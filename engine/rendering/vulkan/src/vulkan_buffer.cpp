#include "vulkan_buffer.h"

#include "vulkan_check.h"

namespace etude::vulkan {

    Buffer createBuffer(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        MemoryAccess access
    ) {
        const VkBufferCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VkBuffer handle = nullptr;
        check(vkCreateBuffer(device, &info, nullptr, &handle), "vkCreateBuffer");

        VkMemoryRequirements requirements{};
        vkGetBufferMemoryRequirements(device, handle, &requirements);
        const bool addressable = (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0;
        const VkMemoryAllocateFlags flags = addressable ? VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT : 0u;

        Buffer buffer{
            .allocation = allocateMemory(physicalDevice, device, requirements, access, flags),
            .handle = BufferHandle(handle, {device}),
            .size = size,
        };
        check(
            vkBindBufferMemory(device, handle, buffer.allocation.memory.get(), buffer.allocation.offset),
            "vkBindBufferMemory"
        );

        if (addressable) {
            const VkBufferDeviceAddressInfo addressInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .buffer = handle,
            };
            buffer.address = vkGetBufferDeviceAddress(device, &addressInfo);
        }

        return buffer;
    }
}
