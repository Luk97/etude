#include <etude/rendering/vulkan/vulkan_renderer.h>

#include "vulkan_check.h"
#include "vulkan_context.h"
#include "vulkan_handles.h"
#include "vulkan_pipeline.h"
#include "vulkan_swapchain.h"
#include "vulkan_sync.h"

#include <etude/core/log.h>
#include <etude/platform/window.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    namespace {

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
            return CommandPool(pool, {device});
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
                const VkFence fence = frame.inFlight.get();
                check(vkWaitForFences(context.device.get(), 1, &fence, VK_TRUE, noTimeout), "vkWaitForFences");

                std::uint32_t imageIndex = 0;
                const VkResult acquired = vkAcquireNextImageKHR(
                    context.device.get(), swapchain->handle.get(), noTimeout, frame.imageAvailable.get(), nullptr,
                    &imageIndex
                );
                if (acquired == VK_ERROR_OUT_OF_DATE_KHR) {
                    destroySwapchain();
                    return;
                }
                if (acquired != VK_SUBOPTIMAL_KHR) {
                    check(acquired, "vkAcquireNextImageKHR");
                }
                check(vkResetFences(context.device.get(), 1, &fence), "vkResetFences");

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

                // Viewport and scissor are dynamic state, so the pipeline stays valid when the window size changes.
                const VkViewport viewport{
                    .width = static_cast<float>(area.extent.width),
                    .height = static_cast<float>(area.extent.height),
                    .maxDepth = 1.0f,
                };
                vkCmdSetViewport(commands, 0, 1, &viewport);
                vkCmdSetScissor(commands, 0, 1, &area);

                // Three vertices without a vertex buffer, the vertex shader looks up each corner by its index.
                vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle.get());
                vkCmdDraw(commands, 3, 1, 0, 0);

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
                check(vkQueueSubmit2(context.queue, 1, &info, frame.inFlight.get()), "vkQueueSubmit2");
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
                return vkQueuePresentKHR(context.queue, &info);
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
