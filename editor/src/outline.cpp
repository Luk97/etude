#include <etude/editor/outline.h>

#include <etude/math/angle.h>

#include <cmath>

namespace etude::editor {

    std::array<Sprite, 4> outline(Vec2 center, Vec2 size, float rotation, float thickness, Color color) {
        const Vec2 half = Vec2{std::abs(size.x), std::abs(size.y)} / 2.0f;
        const float radians = toRadians(rotation);

        // Each edge is a sprite that turns with the rectangle around its own middle, which the turn carries around the
        // center. The top and the bottom edge reach over the corners, so that the corners are filled.
        const auto edge = [&](Vec2 middle, Vec2 extent) {
            return Sprite{
                .position = center + middle.rotated(radians) - extent * 0.5f,
                .size = extent,
                .rotation = radians,
                .color = color,
            };
        };
        const Vec2 across{2.0f * half.x + thickness, thickness};
        const Vec2 along{thickness, 2.0f * half.y};
        return {
            edge({0.0f, -half.y}, across),
            edge({0.0f, half.y}, across),
            edge({-half.x, 0.0f}, along),
            edge({half.x, 0.0f}, along),
        };
    }
}
