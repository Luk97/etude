#include "vulkan_frame.h"

#include "vulkan_command_buffer.h"

namespace etude::vulkan {

    Frame createFrame(VkDevice device, VkCommandPool pool) {
        return {
            .commands = allocateCommandBuffer(device, pool),
            .imageAvailable = createSemaphore(device),
            .inFlight = createFence(device, VK_FENCE_CREATE_SIGNALED_BIT),
        };
    }
}
