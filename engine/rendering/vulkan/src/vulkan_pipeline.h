#pragma once

#include "vulkan_handles.h"

#include <etude/math/mat3.h>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using PipelineLayout = DeviceChild<VkPipelineLayout, vkDestroyPipelineLayout>;
    using PipelineHandle = DeviceChild<VkPipeline, vkDestroyPipeline>;

    /// @brief A graphics pipeline with its layout. The members are destroyed in reverse order, so the pipeline
    /// goes before its layout.
    struct Pipeline {
        PipelineLayout layout;
        PipelineHandle handle;
    };

    /// @brief The push constants of the sprite pipeline: the address of the sprites of the frame and the matrix from
    /// world coordinates to clip space. The shader reads them in the scalar block layout, so the matrix follows the
    /// address without gaps, just like here.
    struct SpriteConstants {
        VkDeviceAddress sprites = 0;
        Mat3 viewProjection;
    };

    static_assert(sizeof(Mat3) == 9 * sizeof(float), "The vertex shader expects no gaps.");

    /// @brief Creates the pipeline that draws sprites with alpha blending into images of the given format. It reaches
    /// the textures through a table with the given layout.
    Pipeline createSpritePipeline(VkDevice device, VkFormat colorFormat, VkDescriptorSetLayout textureTable);
}
