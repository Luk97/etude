#pragma once

#include <etude/math/vec2.h>

namespace etude {

    /// @brief An axis-aligned rectangle from its top-left corner and its size. Because y points down,
    /// the top-left corner has the smallest coordinates.
    struct Rect {
        Vec2 position;
        Vec2 size;

        /// @brief Returns whether the point lies inside. The left and top edges belong to the rectangle, the right and
        /// bottom edges do not, so that two rectangles side by side never share a point.
        constexpr bool contains(Vec2 point) const {
            return point.x >= position.x && point.x < position.x + size.x && point.y >= position.y &&
                   point.y < position.y + size.y;
        }

        constexpr bool operator==(const Rect&) const = default;
    };
}
