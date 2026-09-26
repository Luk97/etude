#pragma once

#include "vulkan_handles.h"

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

    /// @brief Creates the pipeline that draws a triangle with colored corners into images of the given format.
    Pipeline createTrianglePipeline(VkDevice device, VkFormat colorFormat);
}
