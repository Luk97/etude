#pragma once

#include "vulkan_handles.h"

#include <cstdint>
#include <span>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using ShaderModule = DeviceChild<VkShaderModule, vkDestroyShaderModule>;

    /// @brief Creates a shader module from SPIR-V words. A pipeline only needs it while the pipeline is created.
    ShaderModule createShaderModule(VkDevice device, std::span<const std::uint32_t> code);
}
