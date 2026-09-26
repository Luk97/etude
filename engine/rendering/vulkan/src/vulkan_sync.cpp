#include "vulkan_sync.h"

#include "vulkan_check.h"

namespace etude::vulkan {

    Semaphore createSemaphore(VkDevice device) {
        const VkSemaphoreCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        VkSemaphore semaphore = nullptr;
        check(vkCreateSemaphore(device, &info, nullptr, &semaphore), "vkCreateSemaphore");
        return Semaphore(semaphore, {device});
    }

    Fence createFence(VkDevice device, VkFenceCreateFlags flags) {
        const VkFenceCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = flags,
        };

        VkFence fence = nullptr;
        check(vkCreateFence(device, &info, nullptr, &fence), "vkCreateFence");
        return Fence(fence, {device});
    }
}
