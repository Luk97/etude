#pragma once

#include "vulkan_handles.h"
#include "vulkan_sampler.h"

#include <cstdint>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using DescriptorSetLayout = DeviceChild<VkDescriptorSetLayout, vkDestroyDescriptorSetLayout>;
    using DescriptorPool = DeviceChild<VkDescriptorPool, vkDestroyDescriptorPool>;

    /// @brief Most textures the table holds at once.
    inline constexpr std::uint32_t maxTextures = 1024;

    /// @brief The table through which shaders reach every texture by its index: one descriptor set with the nearest
    /// sampler and an array of textures, bound once for all draws. The members are destroyed in reverse order, so the
    /// pool frees the set before the layout and the sampler go.
    struct TextureTable {
        Sampler sampler;
        DescriptorSetLayout layout;
        DescriptorPool pool;
        VkDescriptorSet set = nullptr;
    };

    /// @brief Creates an empty table. Its array may have gaps, because shaders only read the entries that hold a
    /// texture.
    TextureTable createTextureTable(VkDevice device);

    /// @brief Enters the view of a texture into the table at the given index. No frame in flight may use the table
    /// meanwhile.
    void writeTexture(VkDevice device, const TextureTable& table, std::uint32_t index, VkImageView view);
}
