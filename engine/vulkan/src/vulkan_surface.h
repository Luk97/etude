#pragma once

#include <vulkan/vulkan.h>

namespace etude {

    class Window;

    /// @brief Returns the name of the instance extension that creates surfaces for the windows of this platform.
    const char* surfaceExtensionName();

    /// @brief Creates the surface through whcih the renderer shows its images in the window.
    /// Defined in vulkan_surface_win32.cpp, the only file of etude_vulkan than includes windows.h.
    VkSurfaceKHR createSurface(VkInstance instance, const Window& window);
}
