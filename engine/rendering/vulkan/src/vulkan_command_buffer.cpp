#include "vulkan_command_buffer.h"

#include "vulkan_check.h"

namespace etude::vulkan {

    VkCommandBuffer allocateCommandBuffer(VkDevice device, VkCommandPool pool) {
        const VkCommandBufferAllocateInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VkCommandBuffer commands = nullptr;
        check(vkAllocateCommandBuffers(device, &info, &commands), "vkAllocateCommandBuffers");
        return commands;
    }

    void beginCommands(VkCommandBuffer commands) {
        check(vkResetCommandBuffer(commands, 0), "vkResetCommandBuffer");
        const VkCommandBufferBeginInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        check(vkBeginCommandBuffer(commands, &info), "vkBeginCommandBuffer");
    }

    void endCommands(VkCommandBuffer commands) {
        check(vkEndCommandBuffer(commands), "vkEndCommandBuffer");
    }

    void submitCommands(
        VkQueue queue,
        VkCommandBuffer commands,
        const VkSemaphoreSubmitInfo& wait,
        const VkSemaphoreSubmitInfo& signal,
        VkFence fence
    ) {
        const VkCommandBufferSubmitInfo buffer{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = commands,
        };
        const VkSubmitInfo2 info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = &wait,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &buffer,
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &signal,
        };
        check(vkQueueSubmit2(queue, 1, &info, fence), "vkQueueSubmit2");
    }
}
