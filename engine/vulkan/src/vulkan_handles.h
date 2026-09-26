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

    struct SemaphoreDeleter {
        VkDevice device = nullptr;

        void operator()(VkSemaphore semaphore) const {
            vkDestroySemaphore(device, semaphore, nullptr);
        }
    };

    struct FenceDeleter {
        VkDevice device = nullptr;

        void operator()(VkFence fence) const {
            vkDestroyFence(device, fence, nullptr);
        }
    };

    struct CommandPoolDeleter {
        VkDevice device = nullptr;

        void operator()(VkCommandPool pool) const {
            vkDestroyCommandPool(device, pool, nullptr);
        }
    };

    struct ShaderModuleDeleter {
        VkDevice device = nullptr;

        void operator()(VkShaderModule shader) const {
            vkDestroyShaderModule(device, shader, nullptr);
        }
    };

    struct PipelineLayoutDeleter {
        VkDevice device = nullptr;

        void operator()(VkPipelineLayout layout) const {
            vkDestroyPipelineLayout(device, layout, nullptr);
        }
    };

    struct PipelineDeleter {
        VkDevice device = nullptr;

        void operator()(VkPipeline pipeline) const {
            vkDestroyPipeline(device, pipeline, nullptr);
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
    using Semaphore = std::unique_ptr<std::remove_pointer_t<VkSemaphore>, SemaphoreDeleter>;
    using Fence = std::unique_ptr<std::remove_pointer_t<VkFence>, FenceDeleter>;
    using CommandPool = std::unique_ptr<std::remove_pointer_t<VkCommandPool>, CommandPoolDeleter>;
    using ShaderModule = std::unique_ptr<std::remove_pointer_t<VkShaderModule>, ShaderModuleDeleter>;
    using PipelineLayout = std::unique_ptr<std::remove_pointer_t<VkPipelineLayout>, PipelineLayoutDeleter>;
    using Pipeline = std::unique_ptr<std::remove_pointer_t<VkPipeline>, PipelineDeleter>;
}
