#pragma once

#include "vulkan_handles.h"

#include <vulkan/vulkan.h>

namespace etude {

    class Window;
}

namespace etude::vulkan {

    using Surface = InstanceChild<VkSurfaceKHR, vkDestroySurfaceKHR>;

    /// @brief Returns the name of the instance extension that creates surfaces for the windows of this platform.
    const char* surfaceExtensionName();

    /// @brief Creates the surface through which the renderer shows its images in the window.
    /// Defined in vulkan_surface_win32.cpp, the only file of etude_vulkan that includes windows.h.
    Surface createSurface(VkInstance instance, const Window& window);
}
