#pragma once

#include <memory>
#include <type_traits>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    /// @brief Destroys a Vulkan object that has no parent, such as an instance or a device.
    template <typename Handle, auto destroy>
    struct RootDeleter {
        void operator()(Handle handle) const {
            destroy(handle, nullptr);
        }
    };

    /// @brief Destroys a Vulkan object through the parent that created it, such as a surface through its instance or
    /// a semaphore through its device.
    template <typename Parent, typename Handle, auto destroy>
    struct ChildDeleter {
        Parent parent = nullptr;

        void operator()(Handle handle) const {
            destroy(parent, handle, nullptr);
        }
    };

    /// @brief On 64-bit platforms every Vulkan handle is a pointer to an opaque struct, so std::unique_ptr with a
    /// deleter can own it and destroys it automatically.
    template <typename Handle, auto destroy>
    using Root = std::unique_ptr<std::remove_pointer_t<Handle>, RootDeleter<Handle, destroy>>;

    template <typename Handle, auto destroy>
    using InstanceChild = std::unique_ptr<std::remove_pointer_t<Handle>, ChildDeleter<VkInstance, Handle, destroy>>;

    template <typename Handle, auto destroy>
    using DeviceChild = std::unique_ptr<std::remove_pointer_t<Handle>, ChildDeleter<VkDevice, Handle, destroy>>;

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

    using Instance = Root<VkInstance, vkDestroyInstance>;
    using Messenger = std::unique_ptr<std::remove_pointer_t<VkDebugUtilsMessengerEXT>, MessengerDeleter>;
    using Surface = InstanceChild<VkSurfaceKHR, vkDestroySurfaceKHR>;
    using Device = Root<VkDevice, vkDestroyDevice>;
    using SwapchainHandle = DeviceChild<VkSwapchainKHR, vkDestroySwapchainKHR>;
    using ImageView = DeviceChild<VkImageView, vkDestroyImageView>;
    using Semaphore = DeviceChild<VkSemaphore, vkDestroySemaphore>;
    using Fence = DeviceChild<VkFence, vkDestroyFence>;
    using CommandPool = DeviceChild<VkCommandPool, vkDestroyCommandPool>;
    using ShaderModule = DeviceChild<VkShaderModule, vkDestroyShaderModule>;
    using PipelineLayout = DeviceChild<VkPipelineLayout, vkDestroyPipelineLayout>;
    using PipelineHandle = DeviceChild<VkPipeline, vkDestroyPipeline>;
}
