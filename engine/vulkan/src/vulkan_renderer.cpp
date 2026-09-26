#include <etude/vulkan/vulkan_renderer.h>

#include "vulkan_check.h"
#include "vulkan_handles.h"
#include "vulkan_surface.h"
#include "vulkan_swapchain.h"

#include <etude/core/log.h>
#include <etude/platform/window.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string_view>
#include <vector>

#include <vulkan/vulkan.h>

namespace etude {

    namespace {
#ifdef NDEBUG
        constexpr bool validationEnabled = false;
#else
        constexpr bool validationEnabled = true;
#endif

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

        /// @brief Describes which validation messages reach logValidationMessage.
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

        /// @brief A graphics card that meets the requirements of the renderer, with the queue family it draws and
        /// presents with.
        struct Gpu {
            VkPhysicalDevice device = nullptr;
            std::uint32_t queueFamily = 0;
        };

        /// @brief Creates the connection to the Vulkan loader, with the validation layer in debug builds.
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

        /// @brief Creates the messenger that routes validation messages to the log for the lifetime of the instance.
        Messenger createMessenger(VkInstance instance) {
            const auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
            );
            const VkDebugUtilsMessengerCreateInfoEXT info = messengerInfo();

            VkDebugUtilsMessengerEXT messenger = nullptr;
            check(create(instance, &info, nullptr, &messenger), "vkCreateDebugUtilsMessengerEXT");
            return Messenger(messenger, MessengerDeleter{instance});
        }

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

        /// @brief Picks the graphics card to render with. The first dedicated card wins, otherwise the first suitable
        /// one, for example a graphics unit inside the processor.
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
                logFatal(
                    "No graphics card supports Vulkan 1.3 with dynamic rendering, synchronization2 and a swapchain."
                );
                std::abort();
            }

            return *fallback;
        }

        /// @brief Creates the logical device with one queue and the features that the renderer relies on.
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

        class VulkanRenderer : public Renderer {
        public:
            explicit VulkanRenderer(const Window& window) : window(window), instance(createInstance()) {
                if constexpr (validationEnabled) {
                    messenger = createMessenger(instance.get());
                }
                surface = Surface(createSurface(instance.get(), window), SurfaceDeleter{instance.get()});

                gpu = chooseGpu(instance.get(), surface.get());
                device = createDevice(gpu);
                vkGetDeviceQueue(device.get(), gpu.queueFamily, 0, &queue);

                VkPhysicalDeviceProperties properties{};
                vkGetPhysicalDeviceProperties(gpu.device, &properties);
                logInfo(
                    "Vulkan {}.{}.{} ready, validation {}", VK_API_VERSION_MAJOR(properties.apiVersion),
                    VK_API_VERSION_MINOR(properties.apiVersion), VK_API_VERSION_PATCH(properties.apiVersion),
                    validationEnabled ? "on" : "off"
                );
            }

            /// @brief Keeps the swapchain matched to the window. A new window size needs a new swapchain, and a
            /// minimized window gets none, because Vulkan cannot create a swapchain without area.
            void render() override {
                const Size size = window.clientSize();
                if (size.width == 0 || size.height == 0 || (swapchain && swapchain->size == size)) {
                    return;
                }

                // A surface belongs to one swapchain at a time, so the old one has to go before the new one exists.
                swapchain.reset();
                swapchain = createSwapchain(gpu.device, device.get(), surface.get(), size);
                logInfo(
                    "Swapchain {} x {} with {} images", swapchain->size.width, swapchain->size.height,
                    swapchain->images.size()
                );
            }

        private:
            const Window& window;
            Instance instance;
            Messenger messenger;
            Surface surface;
            Gpu gpu;
            Device device;
            VkQueue queue = nullptr;
            std::optional<VulkanSwapchain> swapchain;
        };
    }

    std::unique_ptr<Renderer> createVulkanRenderer(const Window& window) {
        return std::make_unique<VulkanRenderer>(window);
    }
}
