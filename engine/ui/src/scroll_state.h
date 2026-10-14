#pragma once

namespace etude::ui {

    /// @brief What the context keeps about a scrolling element from one frame to the next, in pixels of the screen.
    struct ScrollState {
        /// @brief How far the children are moved up.
        float offset = 0.0f;

        /// @brief How far the children, with the padding below them, reached beyond the bottom of the element in the
        /// last frame, which is as far as they can scroll.
        float limit = 0.0f;
    };
}
