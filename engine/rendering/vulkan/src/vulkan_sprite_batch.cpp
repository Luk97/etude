#include "vulkan_sprite_batch.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <optional>

namespace etude::vulkan {

    static_assert(
        sizeof(Sprite) == 2 * sizeof(Vec2) + sizeof(float) + sizeof(TextureId) + sizeof(Rect) + sizeof(Color),
        "The vertex shader expects no gaps."
    );

    namespace {

        /// @brief Turns the clip rectangle into whole pixels inside the target, or into the whole target without one.
        /// A pixel counts as inside when its middle is.
        VkRect2D toScissor(const std::optional<Rect>& clip, VkExtent2D target) {
            if (!clip) {
                return {
                    .extent = target,
                };
            }

            const auto edge = [](float value, std::uint32_t limit) {
                return static_cast<std::uint32_t>(std::clamp(std::round(value), 0.0f, static_cast<float>(limit)));
            };
            const std::uint32_t left = edge(clip->position.x, target.width);
            const std::uint32_t top = edge(clip->position.y, target.height);
            const std::uint32_t right = edge(clip->position.x + clip->size.x, target.width);
            const std::uint32_t bottom = edge(clip->position.y + clip->size.y, target.height);
            const VkOffset2D offset{
                .x = static_cast<std::int32_t>(left),
                .y = static_cast<std::int32_t>(top),
            };
            return {
                .offset = offset,
                .extent = {
                    .width = right > left ? right - left : 0,
                    .height = bottom > top ? bottom - top : 0,
                },
            };
        }
    }

    void uploadSprites(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        SpriteBatch& batch,
        std::span<const DrawBatch> batches,
        VkExtent2D target
    ) {
        batch.draws.clear();
        std::size_t count = 0;
        for (const DrawBatch& drawBatch : batches) {
            count += drawBatch.sprites.size();
        }
        if (count == 0) {
            return;
        }

        const VkDeviceSize size = count * sizeof(Sprite);
        if (size > batch.instances.size) {
            // Twice the need, so that a slowly growing number of sprites does not ask for a new buffer every frame.
            batch.instances = createBuffer(
                physicalDevice, device, 2 * size, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, MemoryAccess::CpuWrite
            );
        }

        auto* const mapped = static_cast<std::byte*>(batch.instances.allocation.mapped);
        std::uint32_t first = 0;
        for (const DrawBatch& drawBatch : batches) {
            if (drawBatch.sprites.empty()) {
                continue;
            }
            std::memcpy(mapped + first * sizeof(Sprite), drawBatch.sprites.data(), drawBatch.sprites.size_bytes());
            const auto spriteCount = static_cast<std::uint32_t>(drawBatch.sprites.size());
            batch.draws.push_back({
                .first = first,
                .count = spriteCount,
                .viewProjection = drawBatch.viewProjection,
                .scissor = toScissor(drawBatch.clip, target),
            });
            first += spriteCount;
        }
    }

    void recordSprites(
        VkCommandBuffer commands,
        const Pipeline& pipeline,
        VkDescriptorSet textureTable,
        const SpriteBatch& batch
    ) {
        if (batch.draws.empty()) {
            return;
        }

        const VkPipelineLayout layout = pipeline.layout.get();
        vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle.get());
        vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &textureTable, 0, nullptr);
        for (const SpriteDraw& draw : batch.draws) {

            // The shader wants the address of the sprites to be a multiple of 16, which an offset of whole sprites is
            // not. So every draw starts at the beginning of the buffer and skips to its sprites by the first instance.
            const SpriteConstants constants{
                .sprites = batch.instances.address,
                .viewProjection = draw.viewProjection,
            };
            vkCmdPushConstants(commands, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &constants);
            vkCmdSetScissor(commands, 0, 1, &draw.scissor);
            vkCmdDraw(commands, 6, draw.count, 0, draw.first);
        }
    }
}
