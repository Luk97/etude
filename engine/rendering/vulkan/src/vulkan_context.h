#pragma once

#include "vulkan_debug.h"
#include "vulkan_device.h"
#include "vulkan_instance.h"
#include "vulkan_surface.h"

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    /// @brief The Vulkan objects that live as long as the renderer: the connection to the loader, the surface of the
    /// window, the graphics card, the device and its queue. The members are destroyed in reverse order, so the device
    /// goes first and the instance last.
    struct Context {
        Instance instance;
        Messenger messenger;
        Surface surface;
        Gpu gpu;
        Device device;
        VkQueue queue = nullptr;
    };

    /// @brief Creates the context for the window, with validation messages in the log in debug builds.
    Context createContext(const Window& window);
}
