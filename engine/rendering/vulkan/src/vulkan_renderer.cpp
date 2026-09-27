#include <etude/rendering/vulkan/vulkan_renderer.h>

#include "vulkan_buffer.h"
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
#include "vulkan_texture.h"
#include "vulkan_texture_table.h"

#include <etude/core/assert.h>
#include <etude/core/log.h>
#include <etude/platform/window.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    namespace {

        /// @brief The corners of the triangle in clip space, where y points down, with their texture coordinaes and
        /// colors.
        constexpr std::array triangleCorners{
            TriangleVertex{
                .position = {0.0f, -0.5f},
                .uv = {0.5f, 0.0f},
                .color = {.r = 1.0f},
            },
            TriangleVertex{
                .position = {0.5f, 0.5f},
                .uv = {1.0f, 1.0f},
                .color = {.g = 1.0f},
            },
            TriangleVertex{
                .position = {-0.5f, 0.5f},
                .uv = {0.0f, 1.0f},
                .color = {.b = 1.0f},
            },
        };

        class Renderer final : public etude::Renderer {
        public:
            explicit Renderer(const Window& window) : window(window), context(createContext(window)) {
                // The surface keeps its format, so one pipeline serves every swapchain that is created later.
                surfaceFormat = chooseSurfaceFormat(context.gpu.device, context.surface.get());
                textureTable = createTextureTable(context.device.get());
                pipeline =
                    createTrianglePipeline(context.device.get(), surfaceFormat.format, textureTable.layout.get());

                // The CPU writes the corners once, the vertex shader reads them through the address of the buffer.
                triangle = createBuffer(
                    context.gpu.device, context.device.get(), sizeof(triangleCorners),
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, MemoryAccess::CpuWrite
                );
                std::memcpy(triangle.allocation.mapped, triangleCorners.data(), sizeof(triangleCorners));

                commandPool = createCommandPool(context.device.get(), context.gpu.queueFamily);
                uploads = allocateCommandBuffer(context.device.get(), commandPool.get());
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

            /// @brief Uploads the image and enters it into the texture table. The upload leaves the queue idle, so no
            /// frame in flight uses the table while it changes.
            TextureId createTexture(const Image& image) override {
                ETUDE_ASSERT(textures.size() < maxTextures);

                const auto index = static_cast<std::uint32_t>(textures.size());
                textures.push_back(
                    vulkan::createTexture(context.gpu.device, context.device.get(), context.queue, uploads, image)
                );
                writeTexture(context.device.get(), textureTable, index, textures.back().view.get());

                return static_cast<TextureId>(index);
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
            /// and hand it over for presenting. Until sprites arrive, the triangle shows the newest texture and waits
            /// for the first one.
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

                if (!textures.empty()) {
                    drawTriangle(commands, static_cast<std::uint32_t>(textures.size() - 1));
                }

                endRendering(commands);
                transitionToPresent(commands, image);
                endCommands(commands);
            }

            /// @brief Draws the triangle with the texture at the given index of the texture table. Three vertices
            /// without a vertex buffer: the vertex shader fetches each corner by its index from the buffer whose
            /// address it gets as a push constant.
            void drawTriangle(VkCommandBuffer commands, std::uint32_t textureIndex) const {
                const VkPipelineLayout layout = pipeline.layout.get();
                const TriangleConstants constants{
                    .corners = triangle.address,
                    .textureIndex = textureIndex,
                };
                vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle.get());
                vkCmdBindDescriptorSets(
                    commands, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &textureTable.set, 0, nullptr
                );
                vkCmdPushConstants(commands, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &constants);
                vkCmdDraw(commands, 3, 1, 0, 0);
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
            TextureTable textureTable;
            std::vector<Texture> textures;
            Pipeline pipeline;
            Buffer triangle;
            CommandPool commandPool;
            VkCommandBuffer uploads = nullptr;
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
