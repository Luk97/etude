#pragma once

#include <memory>
#include <type_traits>

#include <vulkan/vulkan.h>

namespace etude {

    struct InstanceDeleter {
        void operator()(VkInstance instance) const {
            vkDestroyInstance(instance, nullptr);
        }
    };

    /// @brief Destroys a debug messenger.
    struct MessengerDeleter {
        VkInstance instance = nullptr;

        void operator()(VkDebugUtilsMessengerEXT messenger) const {
            const auto destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
            );
            destroy(instance, messenger, nullptr);
        }
    };

    struct SurfaceDeleter {
        VkInstance instance = nullptr;

        void operator()(VkSurfaceKHR surface) const {
            vkDestroySurfaceKHR(instance, surface, nullptr);
        }
    };

    struct DeviceDeleter {
        void operator()(VkDevice device) const {
            vkDestroyDevice(device, nullptr);
        }
    };

    struct SwapchainDeleter {
        VkDevice device = nullptr;

        void operator()(VkSwapchainKHR swapchain) const {
            vkDestroySwapchainKHR(device, swapchain, nullptr);
        }
    };

    struct ImageViewDeleter {
        VkDevice device = nullptr;

        void operator()(VkImageView view) const {
            vkDestroyImageView(device, view, nullptr);
        }
    };

    /// @brief On 64-bit platforms every Vulkan handle is a pointer to an opaque struct, so std::unique_ptr with
    /// a deleter can own it and destroys it automatically.
    using Instance = std::unique_ptr<std::remove_pointer_t<VkInstance>, InstanceDeleter>;
    using Messenger = std::unique_ptr<std::remove_pointer_t<VkDebugUtilsMessengerEXT>, MessengerDeleter>;
    using Surface = std::unique_ptr<std::remove_pointer_t<VkSurfaceKHR>, SurfaceDeleter>;
    using Device = std::unique_ptr<std::remove_pointer_t<VkDevice>, DeviceDeleter>;
    using SwapchainHandle = std::unique_ptr<std::remove_pointer_t<VkSwapchainKHR>, SwapchainDeleter>;
    using ImageView = std::unique_ptr<std::remove_pointer_t<VkImageView>, ImageViewDeleter>;
}
