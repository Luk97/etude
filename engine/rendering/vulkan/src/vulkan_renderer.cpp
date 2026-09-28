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
#include "vulkan_sprite_batch.h"
#include "vulkan_swapchain.h"
#include "vulkan_texture.h"
#include "vulkan_texture_table.h"

#include <etude/core/assert.h>
#include <etude/core/log.h>
#include <etude/platform/window.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    namespace {

        class Renderer final : public etude::Renderer {
        public:
            explicit Renderer(const Window& window) : window(window), context(createContext(window)) {
                // The surface keeps its format, so one pipeline serves every swapchain that is created later.
                surfaceFormat = chooseSurfaceFormat(context.gpu.device, context.surface.get());
                textureTable = createTextureTable(context.device.get());
                pipeline = createSpritePipeline(context.device.get(), surfaceFormat.format, textureTable.layout.get());

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

            /// @brief Draws the sprites over the clear color into the next swapchain image and presents it.
            void render(const Mat3& viewProjection, std::span<const Sprite> sprites) override {
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

                uploadSprites(context.gpu.device, context.device.get(), frame.sprites, sprites);
                record(frame, *imageIndex, viewProjection);
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

            /// @brief Records the commands for one frame: make the image a color target, clear it, draw the sprites of
            /// the frame and hand the image over for presenting.
            void record(const Frame& frame, std::uint32_t imageIndex, const Mat3& viewProjection) const {
                const VkCommandBuffer commands = frame.commands;
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
                recordSprites(commands, pipeline, textureTable.set, frame.sprites, viewProjection);
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
            TextureTable textureTable;
            std::vector<Texture> textures;
            Pipeline pipeline;
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
