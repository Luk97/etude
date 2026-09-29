#pragma once

#include <etude/math/color.h>
#include <etude/math/rect.h>
#include <etude/math/vec2.h>
#include <etude/rendering/texture_id.h>

namespace etude {

    /// @brief A texture drawn as a rectangle. The position is its top left corner in world coordinates, where y points
    /// down, and the texture stretches over the whole size. The rotation in radians turns the rectangle clockwise
    /// around its middle.
    struct Sprite {
        Vec2 position;
        Vec2 size;
        float rotation = 0.0f;
        TextureId texture{};

        /// @brief The part of the texture that the sprite shows, in texture coordinates from 0 to 1 where y points
        /// down, for example one letter of a font.
        Rect uv{
            .size = {1.0f, 1.0f},
        };

        /// @brief Multiplies the texture, so that white leaves it as it is. With the white texture of TextureId{}, the
        /// sprite becomes a rectangle in this color.
        Color color{
            .r = 1.0f,
            .g = 1.0f,
            .b = 1.0f,
            .a = 1.0f,
        };
    };
}
