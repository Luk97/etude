#pragma once

#include <etude/math/size.h>
#include <etude/math/vec2.h>
#include <etude/render2d/render_list.h>
#include <etude/rendering/texture_id.h>
#include <etude/scene/world.h>

#include <functional>
#include <optional>
#include <string>

namespace etude {

    /// @brief A texture on the GPU with the size of its image in pixels, which its sprites have at scale 1.
    struct LoadedTexture {
        TextureId id{};
        Vec2 size;
    };

    /// @brief Returns the texture for a path relative to the asset folder, or nothing if it cannot be loaded. Passed
    /// in as a function, it lets drawScene run in tests without a GPU.
    using TextureLookup = std::function<std::optional<LoadedTexture>(const std::string& path)>;

    /// @brief Adds a sprite to the list for each entity with a Transform2D and a SpriteRenderer, ordered by layer and
    /// within a layer by entity index, so that later entities of a scene file cover earlier ones. Entities whose
    /// texture cannot be loaded stay invisible. The Camera with the lowest entity index, if there is one, points the
    /// camera of the list and ignores the rotation and scale of its transform. A Camera without a positive zoom is
    /// skipped, since it would divide by zero or mirror the picture.
    void drawScene(const World& world, Size window, const TextureLookup& textures, RenderList& list);
}
