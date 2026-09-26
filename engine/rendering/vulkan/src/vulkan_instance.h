#pragma once

#include "vulkan_handles.h"

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    using Instance = Root<VkInstance, vkDestroyInstance>;

    /// @brief Creates the connection to the Vulkan loader, with the validation layer in debug builds.
    Instance createInstance();
}
