#pragma once

#include <etude/math/vec2.h>

#include <algorithm>

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

        /// @brief Returns the part that both rectangles cover, which has no size if they do not overlap.
        constexpr Rect intersection(Rect other) const {
            const float left = std::max(position.x, other.position.x);
            const float top = std::max(position.y, other.position.y);
            const float right = std::min(position.x + size.x, other.position.x + other.size.x);
            const float bottom = std::min(position.y + size.y, other.position.y + other.size.y);
            return {
                .position = {left, top},
                .size = {std::max(right - left, 0.0f), std::max(bottom - top, 0.0f)},
            };
        }

        constexpr bool operator==(const Rect&) const = default;
    };
}
