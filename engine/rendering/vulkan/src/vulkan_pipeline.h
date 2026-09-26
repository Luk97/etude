#pragma once

#include "vulkan_handles.h"

#include <etude/math/color.h>
#include <etude/math/vec2.h>

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

    /// @brief A corner of the triangle as the vertex shader reads it. The shader uses the scalar block layout, so
    /// the fields follow each other without gaps, just like here.
    struct TriangleVertex {
        Vec2 position;
        Color color;
    };

    static_assert(sizeof(TriangleVertex) == 6 * sizeof(float), "The vertex shader expects the fields without gaps.");

    /// @brief Creates the pipeline that draws a triangle with colored corners into images of the given format.
    Pipeline createTrianglePipeline(VkDevice device, VkFormat colorFormat);
}
