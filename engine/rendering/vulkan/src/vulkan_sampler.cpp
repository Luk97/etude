#include "vulkan_sampler.h"

#include "vulkan_check.h"

namespace etude::vulkan {

    Sampler createNearestSampler(VkDevice device) {
        const VkSamplerCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        };

        VkSampler sampler = nullptr;
        check(vkCreateSampler(device, &info, nullptr, &sampler), "vkCreateSampler");
        return Sampler(sampler, {device});
    }
}
