#pragma once

#include <etude/math/vec2.h>

namespace etude {

    /// @brief An axis-aligned rectangle from its top-left corner and its size. Because y points down,
    /// the top-left corner has the smallest coordinates.
    struct Rect {
        Vec2 position;
        Vec2 size;

        constexpr bool operator==(const Rect&) const = default;
    };
}
