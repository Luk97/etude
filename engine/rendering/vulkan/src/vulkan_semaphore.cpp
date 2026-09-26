#include "vulkan_semaphore.h"

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
}
