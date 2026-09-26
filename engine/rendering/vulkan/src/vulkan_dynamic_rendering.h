#pragma once

#include <etude/core/color.h>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    /// @brief Starts dynamic rendering into a color target that is first cleared with the given color, and points
    /// viewport and scissor at the render area. Both are dynamic state, so a pipeline stays valid when the window size
    /// changes.
    void beginRendering(VkCommandBuffer commands, VkImageView target, VkRect2D area, Color clearColor);

    /// @brief Ends the rendering that beginRendering started.
    void endRendering(VkCommandBuffer commands);
}
