#include "vulkan_fence.h"

#include "vulkan_check.h"

#include <cstdint>
#include <limits>

namespace etude::vulkan {

    namespace {

        constexpr std::uint64_t noTimeout = std::numeric_limits<std::uint64_t>::max();
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

    void waitForFence(VkDevice device, VkFence fence) {
        check(vkWaitForFences(device, 1, &fence, VK_TRUE, noTimeout), "vkWaitForFences");
    }

    void resetFence(VkDevice device, VkFence fence) {
        check(vkResetFences(device, 1, &fence), "vkResetFences");
    }
}
