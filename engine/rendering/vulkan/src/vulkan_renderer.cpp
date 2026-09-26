#include <etude/rendering/vulkan/vulkan_renderer.h>

#include "vulkan_check.h"
#include "vulkan_command_buffer.h"
#include "vulkan_command_pool.h"
#include "vulkan_context.h"
#include "vulkan_dynamic_rendering.h"
#include "vulkan_fence.h"
#include "vulkan_frame.h"
#include "vulkan_image.h"
#include "vulkan_pipeline.h"
#include "vulkan_swapchain.h"

#include <etude/core/log.h>
#include <etude/platform/window.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    namespace {

        class Renderer final : public etude::Renderer {
        public:
            explicit Renderer(const Window& window) : window(window), context(createContext(window)) {
                // The surface keeps its format, so one pipeline serves every swapchain that is created later.
                surfaceFormat = chooseSurfaceFormat(context.gpu.device, context.surface.get());
                pipeline = createTrianglePipeline(context.device.get(), surfaceFormat.format);

                commandPool = createCommandPool(context.device.get(), context.gpu.queueFamily);
                for (Frame& frame : frames) {
                    frame = createFrame(context.device.get(), commandPool.get());
                }
            }

            /// @brief Waits for the GPU first, because the last frames may still use the objects that are destroyed
            /// afterwards.
            ~Renderer() override {
                vkDeviceWaitIdle(context.device.get());
            }

            /// @brief Draws the triangle over the clear color into the next swapchain image and presents it.
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
                waitForFence(context.device.get(), frame.inFlight.get());

                const std::optional<std::uint32_t> imageIndex =
                    acquireImage(context.device.get(), *swapchain, frame.imageAvailable.get());
                if (!imageIndex) {
                    destroySwapchain();
                    return;
                }

                // Reset only now that the frame is sure to be submitted, otherwise the next wait would block forever.
                resetFence(context.device.get(), frame.inFlight.get());

                record(frame.commands, *imageIndex);
                submit(frame, *imageIndex);
                if (!presentImage(context.queue, *swapchain, *imageIndex)) {
                    destroySwapchain();
                }
                frameIndex = (frameIndex + 1) % framesInFlight;
            }

            void setClearColor(Color color) override {
                clearColor = color;
            }

        private:
            void recreateSwapchain(Size size) {
                destroySwapchain();
                swapchain = createSwapchain(
                    context.gpu.device, context.device.get(), context.surface.get(), surfaceFormat, size
                );
                logInfo(
                    "Swapchain {} x {} with {} images", swapchain->size.width, swapchain->size.height,
                    swapchain->images.size()
                );
            }

            /// @brief Destroys the swapchain once the GPU no longer draws into its images, the next frame creates a
            /// new one. A surface belongs to one swapchain at a time, so the old one has to go first.
            void destroySwapchain() {
                check(vkDeviceWaitIdle(context.device.get()), "vkDeviceWaitIdle");
                swapchain.reset();
            }

            /// @brief Records the commands for one frame: make the image a color target, clear it, draw the triangle
            /// and hand it over for presenting.
            void record(VkCommandBuffer commands, std::uint32_t imageIndex) const {
                const VkImage image = swapchain->images[imageIndex];
                const VkRect2D area{
                    .extent = {
                        .width = static_cast<std::uint32_t>(swapchain->size.width),
                        .height = static_cast<std::uint32_t>(swapchain->size.height),
                    },
                };

                beginCommands(commands);
                transitionToColorTarget(commands, image);
                beginRendering(commands, swapchain->views[imageIndex].get(), area, clearColor);

                // Three vertices without a vertex buffer, the vertex shader looks up each corner by its index.
                vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle.get());
                vkCmdDraw(commands, 3, 1, 0, 0);

                endRendering(commands);
                transitionToPresent(commands, image);
                endCommands(commands);
            }

            /// @brief Sends the recorded commands to the GPU. They start drawing once the image is acquired, then
            /// signal renderFinished for presenting and the fence of the frame for the CPU.
            void submit(const Frame& frame, std::uint32_t imageIndex) const {
                const VkSemaphoreSubmitInfo wait{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                    .semaphore = frame.imageAvailable.get(),
                    .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                };
                const VkSemaphoreSubmitInfo signal{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                    .semaphore = swapchain->renderFinished[imageIndex].get(),
                    .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                };
                submitCommands(context.queue, frame.commands, wait, signal, frame.inFlight.get());
            }

            const Window& window;
            Context context;
            VkSurfaceFormatKHR surfaceFormat{};
            Pipeline pipeline;
            CommandPool commandPool;
            std::array<Frame, framesInFlight> frames;
            std::size_t frameIndex = 0;
            std::optional<Swapchain> swapchain;
            Color clearColor;
        };
    }
}

namespace etude {

    std::unique_ptr<Renderer> createVulkanRenderer(const Window& window) {
        return std::make_unique<vulkan::Renderer>(window);
    }
}
