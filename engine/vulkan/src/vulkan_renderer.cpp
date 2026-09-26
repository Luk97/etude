#include <etude/vulkan/vulkan_renderer.h>

#include "vulkan_check.h"
#include "vulkan_handles.h"
#include "vulkan_surface.h"
#include "vulkan_swapchain.h"
#include "vulkan_sync.h"

#include <etude/core/log.h>
#include <etude/platform/window.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
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

        /// @brief The CPU records one frame while the GPU still renders the other. More frames would only add latency.
        constexpr std::size_t framesInFlight = 2;

        /// @brief The objects of one frame in flight. Each frame needs its own, because the GPU may still use those
        /// of the other frame.
        struct Frame {
            VkCommandBuffer commands = nullptr;
            Semaphore imageAvailable;
            Fence inFlight;
        };

        /// @brief Creates the pool for the command buffers. It allows resetting single buffers, because every
        /// frame records its own buffer anew.
        CommandPool createCommandPool(VkDevice device, std::uint32_t queueFamily) {
            const VkCommandPoolCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = queueFamily,
            };

            VkCommandPool pool = nullptr;
            check(vkCreateCommandPool(device, &info, nullptr, &pool), "vkCreateCommandPool");
            return CommandPool(pool, CommandPoolDeleter{device});
        }

        Frame createFrame(VkDevice device, VkCommandPool pool) {
            Frame frame;
            const VkCommandBufferAllocateInfo allocation{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };
            check(vkAllocateCommandBuffers(device, &allocation, &frame.commands), "vkAllocateCommandBuffers");
            frame.imageAvailable = createSemaphore(device);

            // Signaled from the start, so that the first frame does not wait for a frame that never ran.
            frame.inFlight = createFence(device, VK_FENCE_CREATE_SIGNALED_BIT);
            return frame;
        }

        /// @brief The single color layer of a swapchain image.
        constexpr VkImageSubresourceRange colorLayer{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1,
        };

        /// @brief Records a pipeline barrier with a single image barrier.
        void recordImageBarrier(VkCommandBuffer commands, const VkImageMemoryBarrier2& barrier) {
            const VkDependencyInfo dependency{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &barrier,
            };
            vkCmdPipelineBarrier2(commands, &dependency);
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

                commandPool = createCommandPool(device.get(), gpu.queueFamily);
                for (Frame& frame : frames) {
                    frame = createFrame(device.get(), commandPool.get());
                }

                VkPhysicalDeviceProperties properties{};
                vkGetPhysicalDeviceProperties(gpu.device, &properties);
                logInfo(
                    "Vulkan {}.{}.{} ready, validation {}", VK_API_VERSION_MAJOR(properties.apiVersion),
                    VK_API_VERSION_MINOR(properties.apiVersion), VK_API_VERSION_PATCH(properties.apiVersion),
                    validationEnabled ? "on" : "off"
                );
            }

            /// @brief Waits for the GPU first, because the last frames may still use the objects that are destroyed
            /// afterwards.
            ~VulkanRenderer() override {
                vkDeviceWaitIdle(device.get());
            }

            /// @brief Clears the next swapchain image with the clear color and presents it.
            void render() override {
                // A minimized window has no area, and Vulkan cannot create a swapchain without one.
                const Size size = window.clientSize();
                if (size.width == 0 || size.height == 0) {
                    return;
                }
                if (!swapchain || swapchain->size != size) {
                    recreateSwapchain(size);
                }

                // The objects of a frame are free again once the GPU has finished the frame that used them last.
                Frame& frame = frames[frameIndex];
                const VkFence fence = frame.inFlight.get();
                check(vkWaitForFences(device.get(), 1, &fence, VK_TRUE, noTimeout), "vkWaitForFences");

                std::uint32_t imageIndex = 0;
                const VkResult acquired = vkAcquireNextImageKHR(
                    device.get(), swapchain->handle.get(), noTimeout, frame.imageAvailable.get(), nullptr, &imageIndex
                );
                if (acquired == VK_ERROR_OUT_OF_DATE_KHR) {
                    destroySwapchain();
                    return;
                }
                if (acquired != VK_SUBOPTIMAL_KHR) {
                    check(acquired, "vkAcquireNextImageKHR");
                }
                check(vkResetFences(device.get(), 1, &fence), "vkResetFences");

                record(frame.commands, imageIndex);
                submit(frame, imageIndex);

                const VkResult presented = present(imageIndex);
                if (presented == VK_ERROR_OUT_OF_DATE_KHR || presented == VK_SUBOPTIMAL_KHR) {
                    destroySwapchain();
                } else {
                    check(presented, "vkQueuePresentKHR");
                }
                frameIndex = (frameIndex + 1) % framesInFlight;
            }

            void setClearColor(Color color) override {
                clearColor = color;
            }

        private:
            static constexpr std::uint64_t noTimeout = std::numeric_limits<std::uint64_t>::max();

            void recreateSwapchain(Size size) {
                destroySwapchain();
                swapchain = createSwapchain(gpu.device, device.get(), surface.get(), size);
                logInfo(
                    "Swapchain {} x {} with {} images", swapchain->size.width, swapchain->size.height,
                    swapchain->images.size()
                );
            }

            /// @brief Destroys the swapchain once the GPU no longer draws into its images, the next frame creates a
            /// new one. A surface belongs to one swapchain at a time, so the old one has to go first.
            void destroySwapchain() {
                check(vkDeviceWaitIdle(device.get()), "vkDeviceWaitIdle");
                swapchain.reset();
            }

            /// @brief Records the commands for one frame: make the image a color target, clear it and hand it over for
            /// presenting.
            void record(VkCommandBuffer commands, std::uint32_t imageIndex) const {
                check(vkResetCommandBuffer(commands, 0), "vkResetCommandBuffer");
                const VkCommandBufferBeginInfo begin{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                };
                check(vkBeginCommandBuffer(commands, &begin), "vkBeginCommandBuffer");

                // The old content does not matter, so the image goes from UNDEFINED to a color target.
                // The barrier waits in the stage in which the submit waits for the acquired image.
                const VkImage image = swapchain->images[imageIndex];
                const VkImageMemoryBarrier2 toColorTarget{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .image = image,
                    .subresourceRange = colorLayer,
                };
                recordImageBarrier(commands, toColorTarget);

                const VkRenderingAttachmentInfo color{
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = swapchain->views[imageIndex].get(),
                    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = {
                        .color = {
                            .float32 = {clearColor.r, clearColor.g, clearColor.b, clearColor.a},
                        },
                    },
                };
                const VkRect2D area{
                    .extent = {
                        .width = static_cast<std::uint32_t>(swapchain->size.width),
                        .height = static_cast<std::uint32_t>(swapchain->size.height),
                    },
                };
                const VkRenderingInfo rendering{
                    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                    .renderArea = area,
                    .layerCount = 1,
                    .colorAttachmentCount = 1,
                    .pColorAttachments = &color,
                };
                vkCmdBeginRendering(commands, &rendering);
                vkCmdEndRendering(commands);

                // Presenting reads the image outside of the pipeline. The semaphore signaled by the submit orders it
                // after this barrier.
                const VkImageMemoryBarrier2 toPresent{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    .image = image,
                    .subresourceRange = colorLayer,
                };
                recordImageBarrier(commands, toPresent);

                check(vkEndCommandBuffer(commands), "vkEndCommandBuffer");
            }

            /// @brief Sends the recorded commands to the GPU. They start drawing once the image is acquired, then
            /// signal renderFinished for presenting and the fence of the frame for the CPU.
            void submit(const Frame& frame, std::uint32_t imageIndex) const {
                const VkSemaphoreSubmitInfo wait{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                    .semaphore = frame.imageAvailable.get(),
                    .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                };
                const VkCommandBufferSubmitInfo commands{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                    .commandBuffer = frame.commands,
                };
                const VkSemaphoreSubmitInfo signal{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                    .semaphore = swapchain->renderFinished[imageIndex].get(),
                    .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                };
                const VkSubmitInfo2 info{
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                    .waitSemaphoreInfoCount = 1,
                    .pWaitSemaphoreInfos = &wait,
                    .commandBufferInfoCount = 1,
                    .pCommandBufferInfos = &commands,
                    .signalSemaphoreInfoCount = 1,
                    .pSignalSemaphoreInfos = &signal,
                };
                check(vkQueueSubmit2(queue, 1, &info, frame.inFlight.get()), "vkQueueSubmit2");
            }

            /// @brief Shows the image in the window once rendering has finished.
            VkResult present(std::uint32_t imageIndex) const {
                const VkSemaphore wait = swapchain->renderFinished[imageIndex].get();
                const VkSwapchainKHR handle = swapchain->handle.get();
                const VkPresentInfoKHR info{
                    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &wait,
                    .swapchainCount = 1,
                    .pSwapchains = &handle,
                    .pImageIndices = &imageIndex,
                };
                return vkQueuePresentKHR(queue, &info);
            }

            const Window& window;
            Instance instance;
            Messenger messenger;
            Surface surface;
            Gpu gpu;
            Device device;
            VkQueue queue = nullptr;
            CommandPool commandPool;
            std::array<Frame, framesInFlight> frames;
            std::size_t frameIndex = 0;
            std::optional<VulkanSwapchain> swapchain;
            Color clearColor;
        };
    }

    std::unique_ptr<Renderer> createVulkanRenderer(const Window& window) {
        return std::make_unique<VulkanRenderer>(window);
    }
}
