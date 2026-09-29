#include <etude/rendering/vulkan/vulkan_renderer.h>

#include "vulkan_check.h"
#include "vulkan_command_buffer.h"
#include "vulkan_command_pool.h"
#include "vulkan_context.h"
#include "vulkan_dynamic_rendering.h"
#include "vulkan_fence.h"
#include "vulkan_frame.h"
#include "vulkan_gpu_timer.h"
#include "vulkan_image.h"
#include "vulkan_pipeline.h"
#include "vulkan_sprite_batch.h"
#include "vulkan_swapchain.h"
#include "vulkan_texture.h"
#include "vulkan_texture_table.h"

#include <etude/core/assert.h>
#include <etude/core/clock.h>
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

                // Created first, so that TextureId{} names it and a sprite without a texture shows its plain color.
                createTexture({
                    .width = 1,
                    .height = 1,
                    .pixels = {{255, 255, 255, 255}},
                });
            }

            /// @brief Waits for the GPU first, because the last frames may still use the objects that are destroyed
            /// afterwards.
            ~Renderer() override {
                vkDeviceWaitIdle(context.device.get());
            }

            /// @brief Draws the batches into the scene image, then copies it into the window and presents the frame.
            void render(std::span<const DrawBatch> batches) override {
                times = {};

                // A minimized window has no area, and Vulkan cannot create a swapchain without one.
                const Size size = window.clientSize();
                if (size.width == 0 || size.height == 0) {
                    return;
                }
                if (!swapchain || swapchain->size != size) {
                    recreateTargets(size);
                }

                // The objects of a frame are free again once the GPU has finished the frame that used them last. By
                // then it has also written the timestamps of that frame.
                Frame& frame = frames[frameIndex];
                waitForFence(context.device.get(), frame.inFlight.get());
                times.gpu = readTiming(context.device.get(), frame.timer, context.gpu.timestampPeriod);

                const std::optional<std::uint32_t> imageIndex =
                    acquireImage(context.device.get(), *swapchain, frame.imageAvailable.get());
                if (!imageIndex) {
                    destroyTargets();
                    return;
                }

                // From here on the CPU no longer waits, so the clock measures only its work.
                const Clock work;

                // Reset only now that the frame is sure to be submitted, otherwise the next wait would block forever.
                resetFence(context.device.get(), frame.inFlight.get());

                uploadSprites(context.gpu.device, context.device.get(), frame.sprites, batches, targetExtent());
                record(frame, *imageIndex);
                submit(frame, *imageIndex);
                times.cpu = work.elapsed();

                if (!presentImage(context.queue, *swapchain, *imageIndex)) {
                    destroyTargets();
                }
                frameIndex = (frameIndex + 1) % framesInFlight;
            }

            /// @brief Keeps the color in linear light, in which the scene image stores its pixels.
            void setClearColor(Color color) override {
                clearColor = toLinear(color);
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

            FrameTimes frameTimes() const override {
                return times;
            }

        private:
            /// @brief Creates the swapchain for the window and the scene image in the same size and format, so the blit
            /// into the window only copies.
            void recreateTargets(Size size) {
                destroyTargets();
                swapchain = createSwapchain(
                    context.gpu.device, context.device.get(), context.surface.get(), surfaceFormat, size
                );
                sceneImage = createImage(
                    context.gpu.device, context.device.get(), swapchain->size, surfaceFormat.format,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                );
                logInfo(
                    "Swapchain {} x {} with {} images", swapchain->size.width, swapchain->size.height,
                    swapchain->images.size()
                );
            }

            /// @brief Destroys the swapchain and the scene image once the GPU no longer uses them, the next frame
            /// creates new ones. A surface belongs to one swapchain at a time, so the old one has to go first.
            void destroyTargets() {
                check(vkDeviceWaitIdle(context.device.get()), "vkDeviceWaitIdle");
                sceneImage.reset();
                swapchain.reset();
            }

            /// @brief Returns the size of the swapchain, which the scene image shares.
            VkExtent2D targetExtent() const {
                return {
                    .width = static_cast<std::uint32_t>(swapchain->size.width),
                    .height = static_cast<std::uint32_t>(swapchain->size.height),
                };
            }

            /// @brief Records the commands for one frame: draw the sprites into the scene image, then copy it into the
            /// swapchain image and hand that over for presenting. The timer measures only the drawing, because the copy
            /// waits for the display.
            void record(Frame& frame, std::uint32_t imageIndex) const {
                const VkCommandBuffer commands = frame.commands;
                const VkImage scene = sceneImage->handle.get();
                const VkImage target = swapchain->images[imageIndex];
                const VkRect2D area{
                    .extent = targetExtent(),
                };

                beginCommands(commands);
                beginTiming(commands, frame.timer);
                transitionToColorTarget(commands, scene);
                beginRendering(commands, sceneImage->view.get(), area, clearColor);
                recordSprites(commands, pipeline, textureTable.set, frame.sprites);
                endRendering(commands);
                endTiming(commands, frame.timer);

                transitionToBlitSource(commands, scene);
                transitionToBlitTarget(commands, target);
                blitImage(commands, scene, target, swapchain->size);
                transitionToPresent(commands, target);
                endCommands(commands);
            }

            /// @brief Sends the recorded commands to the GPU. The scene is drawn right away, only the blit waits for
            /// the acquired image. At the end they signal renderFinished for presenting and the fence of the frame for
            /// the CPU.
            void submit(const Frame& frame, std::uint32_t imageIndex) const {
                const VkSemaphoreSubmitInfo wait{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                    .semaphore = frame.imageAvailable.get(),
                    .stageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
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
            std::optional<GpuImage> sceneImage;
            FrameTimes times;
            Color clearColor;
        };
    }
}

namespace etude {

    std::unique_ptr<Renderer> createVulkanRenderer(const Window& window) {
        return std::make_unique<vulkan::Renderer>(window);
    }
}
