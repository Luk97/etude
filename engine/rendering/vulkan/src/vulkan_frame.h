#pragma once

#include "vulkan_fence.h"
#include "vulkan_semaphore.h"

#include <cstddef>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    /// @brief The CPU records one frame while the GPU still renders the other. More frames would only add latency.
    inline constexpr std::size_t framesInFlight = 2;

    /// @brief The objects of one frame in flight. Each frame needs its own, because the GPU may still use those of the
    /// other frame.
    struct Frame {
        VkCommandBuffer commands = nullptr;
        Semaphore imageAvailable;
        Fence inFlight;
    };

    /// @brief Creates the objects of one frame. The fence starts signaled, so that the first frame does not wait for a
    /// frame that never ran.
    Frame createFrame(VkDevice device, VkCommandPool pool);
}
