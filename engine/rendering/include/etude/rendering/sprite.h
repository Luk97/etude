#pragma once

#include <etude/math/vec2.h>
#include <etude/rendering/texture_id.h>

namespace etude {

    /// @brief A texture drawn as a rectangle. The position is its top left corner in world coordinates, where y points
    /// down, and the texture stretches over the whole size.
    struct Sprite {
        Vec2 position;
        Vec2 size;
        TextureId texture{};
    };
}
