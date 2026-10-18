#include <etude/runtime/render_system.h>

#include <etude/scene/components/camera.h>
#include <etude/scene/components/sprite_renderer.h>
#include <etude/scene/components/transform2d.h>

#include <algorithm>
#include <cmath>
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

        /// @brief Returns the size of the sprite of an entity: the size of its texture stretched by its scale, negative
        /// where the scale mirrors it.
        Vec2 spriteSize(const LoadedTexture& texture, const Transform2D& transform) {
            return {texture.size.x * transform.scale.x, texture.size.y * transform.scale.y};
        }
    }

    void drawScene(const World& world, Vec2 viewport, const TextureLookup& textures, RenderList& list) {
        // The Camera with the lowest index points the camera of the list, so that its entity lies in the middle of the
        // viewport. Without one, the camera of the list stays as it is.
        std::optional<std::uint32_t> cameraIndex;
        world.each<Camera, Transform2D>([&](Entity entity, const Camera& camera, const Transform2D& transform) {
            // Written this way, it also skips a zoom that is NaN.
            if (!(camera.zoom > 0.0f)) {
                return;
            }
            if (cameraIndex && *cameraIndex < indexOf(entity)) {
                return;
            }
            cameraIndex = indexOf(entity);
            list.camera = {
                .position = transform.position - viewport * (0.5f / camera.zoom),
                .zoom = camera.zoom,
            };
        });

        // Each entity with a texture that loads becomes a sprite with the middle on its position, which notes its layer
        // and index for the sorting.
        std::vector<DrawEntry> entries;
        const auto addSprite = [&](Entity entity, const Transform2D& transform, const SpriteRenderer& spriteRenderer) {
            const std::optional<LoadedTexture> texture = textures(spriteRenderer.texture);
            if (!texture) {
                return;
            }
            const Vec2 size = spriteSize(*texture, transform);
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

        // Sorted by layer and within a layer by index, later sprites go to the list and cover earlier ones.
        std::ranges::sort(entries, {}, [](const DrawEntry& entry) { return std::pair{entry.layer, entry.index}; });
        for (const DrawEntry& entry : entries) {
            list.sprites.push_back(entry.sprite);
        }
    }

    std::optional<Entity> entityAt(const World& world, Vec2 point, const TextureLookup& textures) {
        std::optional<Entity> found;
        int foundLayer = 0;
        const auto test = [&](Entity entity, const Transform2D& transform, const SpriteRenderer& spriteRenderer) {
            // Only an entity that drawScene draws above the one found so far can take its place.
            if (found && std::pair{spriteRenderer.layer, indexOf(entity)} < std::pair{foundLayer, indexOf(*found)}) {
                return;
            }
            const std::optional<LoadedTexture> texture = textures(spriteRenderer.texture);
            if (!texture) {
                return;
            }

            // Turning the point back around the middle of the sprite lines it up with the unturned rectangle, which
            // only has to be compared with half the size.
            const Vec2 offset = point - transform.position;
            const float radians = transform.rotation * radiansPerDegree;
            const float cosine = std::cos(radians);
            const float sine = std::sin(radians);
            const Vec2 local{offset.x * cosine + offset.y * sine, offset.y * cosine - offset.x * sine};
            const Vec2 size = spriteSize(*texture, transform);
            if (std::abs(local.x) <= std::abs(size.x) / 2.0f && std::abs(local.y) <= std::abs(size.y) / 2.0f) {
                found = entity;
                foundLayer = spriteRenderer.layer;
            }
        };
        world.each<Transform2D, SpriteRenderer>(test);
        return found;
    }
}
