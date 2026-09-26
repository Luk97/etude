#pragma once

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    /// @brief Allocates a primary command buffer from the pool. The pool frees it when it is destroyed.
    VkCommandBuffer allocateCommandBuffer(VkDevice device, VkCommandPool pool);

    /// @brief Resets the command buffer and starts recording into it for a single submit.
    void beginCommands(VkCommandBuffer commands);

    /// @brief Ends the recording, after which the command buffer can be submitted.
    void endCommands(VkCommandBuffer commands);

    /// @brief Sends the command buffer to the queue. It starts once wait is signaled and signals signal when it is
    /// done, and the fence for the CPU.
    void submitCommands(
        VkQueue queue,
        VkCommandBuffer commands,
        const VkSemaphoreSubmitInfo& wait,
        const VkSemaphoreSubmitInfo& signal,
        VkFence fence
    );
}
