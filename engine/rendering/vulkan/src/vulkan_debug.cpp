#include "vulkan_debug.h"

#include "vulkan_check.h"

#include <etude/core/log.h>

namespace etude::vulkan {

    namespace {

        /// @brief Forwards warnings and errors of the validation layer to the ETUDE log.
        VKAPI_ATTR VkBool32 VKAPI_CALL logValidationMessage(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT,
            const VkDebugUtilsMessengerCallbackDataEXT* data,
            void*
        ) {
            if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
                logError("Vulkan: {}", data->pMessage);
            } else {
                logWarning("Vulkan: {}", data->pMessage);
            }
            return VK_FALSE;
        }
    }

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = logValidationMessage,
        };
    }

    Messenger createMessenger(VkInstance instance) {
        const auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
        );
        const VkDebugUtilsMessengerCreateInfoEXT info = messengerInfo();

        VkDebugUtilsMessengerEXT messenger = nullptr;
        check(create(instance, &info, nullptr, &messenger), "vkCreateDebugUtilsMessengerEXT");
        return Messenger(messenger, {instance});
    }
}
