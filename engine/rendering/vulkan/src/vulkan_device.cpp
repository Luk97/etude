#include "vulkan_device.h"

#include "vulkan_check.h"

#include <etude/core/log.h>

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <string_view>
#include <vector>

namespace etude::vulkan {

    namespace {
        /// @brief Returns the first queue family of the device that can both draw and present to the surface.
        std::optional<std::uint32_t> findQueueFamily(VkPhysicalDevice device, VkSurfaceKHR surface) {
            std::uint32_t count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
            std::vector<VkQueueFamilyProperties> families(count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

            for (std::uint32_t family = 0; family < count; ++family) {
                VkBool32 presents = VK_FALSE;
                check(
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, family, surface, &presents),
                    "vkGetPhysicalDeviceSurfaceSupportKHR"
                );
                if ((families[family].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && presents == VK_TRUE) {
                    return family;
                }
            }
            return std::nullopt;
        }

        /// @brief Returns true if the device supports Vulkan 1.3 with dynamic rendering and synchronization2, and the
        /// swapchain extension that shows images in a window.
        bool meetsRequirements(VkPhysicalDevice device) {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(device, &properties);
            if (properties.apiVersion < VK_API_VERSION_1_3) {
                return false;
            }

            std::uint32_t count = 0;
            check(
                vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr),
                "vkEnumerateDeviceExtensionProperties"
            );
            std::vector<VkExtensionProperties> extensions(count);
            check(
                vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data()),
                "vkEnumerateDeviceExtensionProperties"
            );
            const bool swapchain = std::ranges::any_of(extensions, [](const VkExtensionProperties& extension) {
                return std::string_view(extension.extensionName) == VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            });

            VkPhysicalDeviceVulkan13Features features13{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            };
            VkPhysicalDeviceFeatures2 features{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = &features13,
            };
            vkGetPhysicalDeviceFeatures2(device, &features);
            return swapchain && features13.dynamicRendering == VK_TRUE && features13.synchronization2 == VK_TRUE;
        }
    }

    Gpu chooseGpu(VkInstance instance, VkSurfaceKHR surface) {
        std::uint32_t count = 0;
        check(vkEnumeratePhysicalDevices(instance, &count, nullptr), "vkEnumeratePhysicalDevices");
        std::vector<VkPhysicalDevice> devices(count);
        check(vkEnumeratePhysicalDevices(instance, &count, devices.data()), "vkEnumeratePhysicalDevices");

        std::optional<Gpu> fallback;
        for (VkPhysicalDevice device : devices) {
            const std::optional<std::uint32_t> family = findQueueFamily(device, surface);
            if (!family || !meetsRequirements(device)) {
                continue;
            }

            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(device, &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                return {device, *family};
            }
            if (!fallback) {
                fallback = Gpu{device, *family};
            }
        }

        if (!fallback) {
            logFatal("No graphics card supports Vulkan 1.3 with dynamic rendering, synchronization2 and a swapchain.");
            std::abort();
        }

        return *fallback;
    }

    Device createDevice(const Gpu& gpu) {
        const float priority = 1.0f;
        const VkDeviceQueueCreateInfo queue{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = gpu.queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &priority,
        };
        const VkPhysicalDeviceVulkan13Features features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
        };
        const char* const extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        const VkDeviceCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queue,
            .enabledExtensionCount = 1,
            .ppEnabledExtensionNames = extensions,
        };

        VkDevice device = nullptr;
        check(vkCreateDevice(gpu.device, &info, nullptr, &device), "vkCreateDevice");
        return Device(device);
    }
}
