#pragma once

#include <etude/math/vec2.h>

namespace etude::ui {

    /// @brief What the context keeps about a floating element from one frame to the next, in pixels of the screen.
    struct LayerState {
        /// @brief Where the floating element sits, measured from the top left corner of the screen.
        Vec2 position;

        /// @brief Where the mouse grabbed the drag handle, measured from the position.
        Vec2 grab;
    };
}
