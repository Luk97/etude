#pragma once

#include <etude/rendering/renderer.h>

#include <memory>

namespace etude {

    class Window;

    /// @brief Creates the Vulkan implementation of the renderer. All Vulkan types stay inside etude_vulkan.
    std::unique_ptr<Renderer> createVulkanRenderer(const Window& window);
}
