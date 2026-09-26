#include "vulkan_command_pool.h"

#include "vulkan_check.h"

namespace etude::vulkan {

    CommandPool createCommandPool(VkDevice device, std::uint32_t queueFamily) {
        const VkCommandPoolCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queueFamily,
        };

        VkCommandPool pool = nullptr;
        check(vkCreateCommandPool(device, &info, nullptr, &pool), "vkCreateCommandPool");
        return CommandPool(pool, {device});
    }
}
