#include "vulkan_gpu_timer.h"

#include "vulkan_check.h"

#include <array>
#include <cstdint>

namespace etude::vulkan {

    GpuTimer createGpuTimer(VkDevice device) {
        const VkQueryPoolCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
            .queryType = VK_QUERY_TYPE_TIMESTAMP,
            .queryCount = 2,
        };

        VkQueryPool pool = nullptr;
        check(vkCreateQueryPool(device, &info, nullptr, &pool), "vkCreateQueryPool");
        return {
            .timestamps = QueryPool(pool, {device}),
        };
    }

    void beginTiming(VkCommandBuffer commands, GpuTimer& timer) {
        vkCmdResetQueryPool(commands, timer.timestamps.get(), 0, 2);
        vkCmdWriteTimestamp2(commands, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, timer.timestamps.get(), 0);
        timer.recorded = true;
    }

    void endTiming(VkCommandBuffer commands, const GpuTimer& timer) {
        vkCmdWriteTimestamp2(commands, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, timer.timestamps.get(), 1);
    }

    std::chrono::nanoseconds readTiming(VkDevice device, const GpuTimer& timer, float timestampPeriod) {
        if (!timer.recorded) {
            return {};
        }

        std::array<std::uint64_t, 2> ticks{};
        check(
            vkGetQueryPoolResults(
                device, timer.timestamps.get(), 0, 2, sizeof(ticks), ticks.data(), sizeof(std::uint64_t),
                VK_QUERY_RESULT_64_BIT
            ),
            "vkGetQueryPoolResults"
        );
        const double nanoseconds = static_cast<double>(ticks[1] - ticks[0]) * timestampPeriod;
        return std::chrono::nanoseconds(static_cast<std::int64_t>(nanoseconds));
    }
}
