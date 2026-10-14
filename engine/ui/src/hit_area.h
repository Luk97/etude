#pragma once

#include "id.h"

#include <etude/math/rect.h>

namespace etude::ui {

    /// @brief A part of the last picture that the mouse can hit, kept for every clickable and every scrolling element.
    struct HitArea {
        /// @brief The element that the area belongs to.
        Id id;

        /// @brief The part of the element that no clip rectangle cut off, in pixels of the screen, so that the mouse
        /// misses what is cut off.
        Rect visible;

        /// @brief The floating element that the element lies in, Id{} outside of them, so that only the areas of the
        /// layer in front react to the mouse.
        Id layer;
    };
}
