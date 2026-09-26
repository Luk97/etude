#pragma once

#include <etude/core/log.h>

#include <cstdlib>
#include <string_view>

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

namespace etude::vulkan {

    /// @brief Logs a fatal error and aborts the program if a Vulkan call did not succeed.
    inline void check(VkResult result, std::string_view call) {
        if (result != VK_SUCCESS) {
            logFatal("{} failed with {}", call, string_VkResult(result));
            std::abort();
        }
    }
}
