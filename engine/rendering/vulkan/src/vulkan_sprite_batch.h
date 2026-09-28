#pragma once

#include "vulkan_buffer.h"
#include "vulkan_pipeline.h"

#include <etude/math/mat3.h>
#include <etude/rendering/sprite.h>

#include <cstdint>
#include <span>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    /// @brief The sprites of one frame in flight on the GPU, in a buffer that grows with their number.
    struct SpriteBatch {
        Buffer instances;
        std::uint32_t count = 0;
    };

    /// @brief Copies the sprites into the batch and replaces its buffer with a larger one if they do not fit. Only call
    /// it after the fence of the frame, because the old buffer is destroyed at once.
    void uploadSprites(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        SpriteBatch& batch,
        std::span<const Sprite> sprites
    );

    /// @brief Records one instanced draw for all sprites of the batch. The vertex shader builds six vertices per sprite
    /// and moves them with the matrix from world coordinates to clip space.
    void recordSprites(
        VkCommandBuffer commands,
        const Pipeline& pipeline,
        VkDescriptorSet textureTable,
        const SpriteBatch& batch,
        const Mat3& viewProjection
    );
}
