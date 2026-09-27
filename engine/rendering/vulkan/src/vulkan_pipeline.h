#pragma once

#include "vulkan_handles.h"

#include <etude/math/color.h>
#include <etude/math/vec2.h>

#include <cstdint>

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

    /// @brief A corner of the triangle as the vertex shader reads it, with its position in the texture. The shader
    /// uses the scalar block layout, so the fields follow each other without gaps, just like here.
    struct TriangleVertex {
        Vec2 position;
        Vec2 uv;
        Color color;
    };

    static_assert(sizeof(TriangleVertex) == 8 * sizeof(float), "The vertex shader expects the fields without gaps.");

    /// @brief The push constants of the triangle: the address of the buffer with its corners and the index of its
    /// texture in the texture table.
    struct TriangleConstants {
        VkDeviceAddress corners = 0;
        std::uint32_t textureIndex = 0;
    };

    /// @brief Creates the pipeline that draws a textured triangle with colored corners into images of the given format.
    /// It reaches the textures through a table with the given layout.
    Pipeline createTrianglePipeline(VkDevice device, VkFormat colorFormat, VkDescriptorSetLayout textureTable);
}
