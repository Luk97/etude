#include "vulkan_instance.h"

#include "vulkan_check.h"
#include "vulkan_debug.h"
#include "vulkan_surface.h"

#include <cstdint>
#include <vector>

namespace etude::vulkan {

    Instance createInstance() {
        const VkApplicationInfo application{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "ETUDE",
            .pEngineName = "ETUDE",
            .apiVersion = VK_API_VERSION_1_3,
        };
        const char* const layers[] = {"VK_LAYER_KHRONOS_validation"};

        // Surfaces need two instance extensions: a general one and one for the window system of the platform.
        std::vector<const char*> extensions{VK_KHR_SURFACE_EXTENSION_NAME, surfaceExtensionName()};
        if constexpr (validationEnabled) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // Chained into the create info, the messenger also reports problems of vkCreateInstance itself.
        const VkDebugUtilsMessengerCreateInfoEXT messenger = messengerInfo();
        const VkInstanceCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = validationEnabled ? &messenger : nullptr,
            .pApplicationInfo = &application,
            .enabledLayerCount = validationEnabled ? 1u : 0u,
            .ppEnabledLayerNames = layers,
            .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        VkInstance instance = nullptr;
        check(vkCreateInstance(&info, nullptr, &instance), "vkCreateInstance");
        return Instance(instance);
    }
}
