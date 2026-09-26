#pragma once

#include <memory>
#include <type_traits>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    /// @brief Validation checks every Vulkan call but costs time, so only debug builds switch it on.
#ifdef NDEBUG
    inline constexpr bool validationEnabled = false;
#else
    inline constexpr bool validationEnabled = true;
#endif

    /// @brief Destroys a debug messenger. Its destroy function belongs to an extension, so it is looked up at runtime.
    struct MessengerDeleter {
        VkInstance instance = nullptr;

        void operator()(VkDebugUtilsMessengerEXT messenger) const {
            const auto destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
            );
            destroy(instance, messenger, nullptr);
        }
    };

    using Messenger = std::unique_ptr<std::remove_pointer_t<VkDebugUtilsMessengerEXT>, MessengerDeleter>;

    /// @brief Describes which validation messages reach the log.
    VkDebugUtilsMessengerCreateInfoEXT messengerInfo();

    /// @brief Creates the messenger that routes validation messages to the log for the lifetime of the instance.
    Messenger createMessenger(VkInstance instance);
}
