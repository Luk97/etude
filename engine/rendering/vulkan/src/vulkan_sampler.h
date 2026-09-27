#pragma once

#include "vulkan_handles.h"

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using Sampler = DeviceChild<VkSampler, vkDestroySampler>;

    /// @brief Creates a sampler that takes the nearest texel without blending its neighbors, so that pixel art stays
    /// sharp at any size. Coordinates outside of the texture take the texel at its edge.
    Sampler createNearestSampler(VkDevice device);
}
