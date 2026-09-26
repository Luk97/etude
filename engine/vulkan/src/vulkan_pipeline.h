#pragma once

#include "vulkan_handles.h"

#include <vulkan/vulkan.h>

namespace etude {

    /// @brief A graphics pipeline with its layout. The members are destroyed in reverse order, so the pipeline
    /// goes before its layout.
    struct VulkanPipeline {
        PipelineLayout layout;
        Pipeline handle;
    };

    /// @brief Creates the pipeline that draws a triangle with colored corners into images of the given format.
    VulkanPipeline createTrianglePipeline(VkDevice device, VkFormat colorFormat);
}
