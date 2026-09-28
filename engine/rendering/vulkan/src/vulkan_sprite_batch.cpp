#include "vulkan_sprite_batch.h"

#include <cstring>

namespace etude::vulkan {

    static_assert(sizeof(Sprite) == 2 * sizeof(Vec2) + sizeof(TextureId), "The vertex shader expects no gaps.");

    void uploadSprites(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        SpriteBatch& batch,
        std::span<const Sprite> sprites
    ) {
        batch.count = static_cast<std::uint32_t>(sprites.size());
        if (sprites.empty()) {
            return;
        }

        if (sprites.size_bytes() > batch.instances.size) {
            // Twice the need, so that a slowly growing number of sprites does not ask for a new buffer every frame.
            batch.instances = createBuffer(
                physicalDevice, device, 2 * sprites.size_bytes(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                MemoryAccess::CpuWrite
            );
        }

        std::memcpy(batch.instances.allocation.mapped, sprites.data(), sprites.size_bytes());
    }

    void recordSprites(
        VkCommandBuffer commands,
        const Pipeline& pipeline,
        VkDescriptorSet textureTable,
        const SpriteBatch& batch,
        const Mat3& viewProjection
    ) {
        if (batch.count == 0) {
            return;
        }

        const VkPipelineLayout layout = pipeline.layout.get();
        const SpriteConstants constants{
            .sprites = batch.instances.address,
            .viewProjection = viewProjection,
        };
        vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle.get());
        vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &textureTable, 0, nullptr);
        vkCmdPushConstants(commands, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &constants);
        vkCmdDraw(commands, 6, batch.count, 0, 0);
    }
}
