#include <etude/vulkan/vulkan_renderer.h>

#include <etude/core/log.h>

#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <type_traits>

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

namespace etude {

    namespace {
#ifdef NDEBUG
        constexpr bool validationEnabled = false;
#else
        constexpr bool validationEnabled = true;
#endif

        /// @brief Logs a fatal error and aborts the program if a Vulkan call did not succeed.
        void check(VkResult result, std::string_view call) {
            if (result != VK_SUCCESS) {
                logFatal("{} failed with {}", call, string_VkResult(result));
            }
        }

        /// @brief Forwards warnings and errors of the validatino layer to the ETUDE log.
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
                .pfnUserCallback = logValidationMessage
            };
        }

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

        /// @brief On 64-bit platforms every Vulkan handle is a pointer to an opaque struct, so std::unique_ptr with
        /// a deleter can own it and destroys it automatically.
        using Instance = std::unique_ptr<std::remove_pointer_t<VkInstance>, InstanceDeleter>;
        using Messenger = std::unique_ptr<std::remove_pointer_t<VkDebugUtilsMessengerEXT>, MessengerDeleter>;

        /// @brief Creates the connection to the Vulkan loader, with the validation layer in debug builds.
        Instance createInstance() {
            const VkApplicationInfo application{
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pApplicationName = "ETUDE",
                .pEngineName = "ETUDE",
                .apiVersion = VK_API_VERSION_1_3
            };
            const char* const layers[] = {"VK_LAYER_KHRONOS_validation"};
            const char* const extensions[] = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

            // Chained into the create info, the messenger also reports problems of vkCreateInstance itself.
            const VkDebugUtilsMessengerCreateInfoEXT messenger = messengerInfo();
            const VkInstanceCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext = validationEnabled ? &messenger : nullptr,
                .pApplicationInfo = &application,
                .enabledLayerCount = validationEnabled ? 1u : 0u,
                .ppEnabledLayerNames = layers,
                .enabledExtensionCount = validationEnabled ? 1u : 0u,
                .ppEnabledExtensionNames = extensions
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

        class VulkanRenderer : public Renderer {
        public:
            VulkanRenderer() : instance(createInstance()) {
                if constexpr (validationEnabled) {
                    messenger = createMessenger(instance.get());
                }

                std::uint32_t version = 0;
                check(vkEnumerateInstanceVersion(&version), "vkEnumerateInstanceVersion");
                logInfo(
                    "Vulkan {}.{}.{} ready, validation {}", VK_API_VERSION_MAJOR(version),
                    VK_API_VERSION_MINOR(version), VK_API_VERSION_PATCH(version), validationEnabled ? "on" : "off"
                );
            }

        private:
            Instance instance;
            Messenger messenger;
        };
    }

    std::unique_ptr<Renderer> createVulkanRenderer() {
        return std::make_unique<VulkanRenderer>();
    }
}
