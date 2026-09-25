#pragma once

#include <etude/gfx/renderer.h>

#include <memory>

namespace etude {

    /// @brief Creates the Vulkan implementation of the renderer. All Vulkan types stay inside etude_vulkan.
    std::unique_ptr<Renderer> createVulkanRenderer();
}
