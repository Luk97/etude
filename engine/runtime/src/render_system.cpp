#include <etude/runtime/render_system.h>

#include <etude/scene/components.h>

#include <algorithm>
#include <cstdint>
#include <numbers>
#include <utility>
#include <vector>

namespace etude {

    namespace {

        constexpr float radiansPerDegree = std::numbers::pi_v<float> / 180.0f;

        /// @brief A sprite together with what decides its place in the drawing order.
        struct DrawEntry {
            int layer = 0;
            std::uint32_t index = 0;
            Sprite sprite;
        };
    }

    void drawScene(const World& world, Size window, const TextureLookup& textures, RenderList& list) {
        std::optional<std::uint32_t> cameraIndex;
        world.each<Camera, Transform2D>([&](Entity entity, const Camera& camera, const Transform2D& transform) {
            if (cameraIndex && *cameraIndex < indexOf(entity)) {
                return;
            }
            cameraIndex = indexOf(entity);
            const Vec2 windowSize{static_cast<float>(window.width), static_cast<float>(window.height)};
            list.camera = {
                .position = transform.position - windowSize * (0.5f / camera.zoom),
                .zoom = camera.zoom,
            };
        });

        std::vector<DrawEntry> entries;
        const auto addSprite = [&](Entity entity, const Transform2D& transform, const SpriteRenderer& spriteRenderer) {
            const std::optional<LoadedTexture> texture = textures(spriteRenderer.texture);
            if (!texture) {
                return;
            }
            const Vec2 size{texture->size.x * transform.scale.x, texture->size.y * transform.scale.y};
            entries.push_back({
                .layer = spriteRenderer.layer,
                .index = indexOf(entity),
                .sprite = {
                    .position = transform.position - size * 0.5f,
                    .size = size,
                    .rotation = transform.rotation * radiansPerDegree,
                    .texture = texture->id,
                },
            });
        };
        world.each<Transform2D, SpriteRenderer>(addSprite);
        std::ranges::sort(entries, {}, [](const DrawEntry& entry) { return std::pair{entry.layer, entry.index}; });
        for (const DrawEntry& entry : entries) {
            list.sprites.push_back(entry.sprite);
        }
    }
}
