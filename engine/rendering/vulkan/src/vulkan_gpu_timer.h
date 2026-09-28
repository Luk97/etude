#pragma once

#include "vulkan_handles.h"

#include <chrono>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using QueryPool = DeviceChild<VkQueryPool, vkDestroyQueryPool>;

    /// @brief Measures how long the GPU works on part of a frame, with one timestamp at the start and one at the end.
    struct GpuTimer {
        QueryPool timestamps;

        /// @brief Whether a command buffer has written the timestamps yet. Before that, they hold no values.
        bool recorded = false;
    };

    /// @brief Creates a timer with a pool of two timestamp queries.
    GpuTimer createGpuTimer(VkDevice device);

    /// @brief Records the start of the measurement: resets the timestamps and writes the first one. It has to come
    /// before any rendering in the command buffer, because a query pool cannot be reset during rendering.
    void beginTiming(VkCommandBuffer commands, GpuTimer& timer);

    /// @brief Records the end of the measurement, a timestamp that the GPU writes once all earlier commands are done.
    void endTiming(VkCommandBuffer commands, const GpuTimer& timer);

    /// @brief Returns the time between the two timestamps, or zero before they were first recorded. Only call it after
    /// the fence of the frame, when the GPU has written them.
    std::chrono::nanoseconds readTiming(VkDevice device, const GpuTimer& timer, float timestampPeriod);
}
