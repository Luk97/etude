#pragma once

#include "vulkan_handles.h"

#include <cstdint>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using Device = Root<VkDevice, vkDestroyDevice>;

    /// @brief A graphics card that meets the requirements of the renderer, with the queue family it draws and presents
    /// with.
    struct Gpu {
        VkPhysicalDevice device = nullptr;
        std::uint32_t queueFamily = 0;
    };

    /// @brief Picks the graphics card to render with. The first dedicated card wins, otherwise the first suitable one,
    /// for example a graphics unit inside the processor.
    Gpu chooseGpu(VkInstance instance, VkSurfaceKHR surface);

    /// @brief Creates the logical device with one queue and the features that the renderer relies on.
    Device createDevice(const Gpu& gpu);
}
