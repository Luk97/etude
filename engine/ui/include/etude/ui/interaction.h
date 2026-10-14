#pragma once

#include <etude/math/rect.h>

namespace etude::ui {

    /// @brief What the mouse and the keyboard do with an element in this frame.
    ///
    /// The mouse is tested against the rectangle that the element had in the last frame, because the rectangles of the
    /// current frame are only known after the layout pass. Only elements with a key keep a rectangle, and only
    /// clickable elements react to the mouse, so for other elements everything but the rectangle stays false.
    struct Interaction {
        /// @brief The rectangle of the element in the last frame, in pixels of the screen from its top left corner.
        /// Empty in the first frame of an element and for elements without a key.
        Rect rect;

        /// @brief The mouse is over the element, no other clickable element lies above it, and no other element is
        /// held.
        bool hovered = false;

        /// @brief The left button went down over the element in this frame.
        bool pressed = false;

        /// @brief The left button went down over the element and is still down, wherever the mouse has moved since.
        bool held = false;

        /// @brief The left button came up over the element after it had gone down over it.
        bool clicked = false;

        /// @brief The element has the focus and takes the keyboard.
        bool focused = false;
    };
}
