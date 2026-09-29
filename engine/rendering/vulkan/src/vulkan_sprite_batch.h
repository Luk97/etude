#pragma once

#include "vulkan_buffer.h"
#include "vulkan_pipeline.h"

#include <etude/math/mat3.h>
#include <etude/rendering/draw_batch.h>

#include <cstdint>
#include <span>
#include <vector>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    /// @brief One draw of a frame: which sprites in the buffer it covers, the matrix for them and the scissor that
    /// clips them.
    struct SpriteDraw {
        std::uint32_t first = 0;
        std::uint32_t count = 0;
        Mat3 viewProjection;
        VkRect2D scissor{};
    };

    /// @brief The sprites of one frame in flight on the GPU, those of all batches one after another in a buffer that
    /// grows with their number, and one draw per batch.
    struct SpriteBatch {
        Buffer instances;
        std::vector<SpriteDraw> draws;
    };

    /// @brief Copies the sprites of the batches into the batch and replaces its buffer with a larger one if they do not
    /// fit. The clip rectangles become scissors inside a target of the given size. Only call it after the fence of the
    /// frame, because the old buffer is destroyed at once.
    void uploadSprites(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        SpriteBatch& batch,
        std::span<const DrawBatch> batches,
        VkExtent2D target
    );

    /// @brief Records one instanced draw per batch. The vertex shader builds six vertices per sprite and moves them
    /// with the matrix of the batch to clip space.
    void recordSprites(
        VkCommandBuffer commands,
        const Pipeline& pipeline,
        VkDescriptorSet textureTable,
        const SpriteBatch& batch
    );
}
