#pragma once

#include <etude/ui/interaction.h>

#include <string_view>

namespace etude::ui {

    class Context;

    /// @brief Shows a number with the visible text of the label before it. Dragging the label to the left or the right
    /// changes the value by the step for each pixel that the mouse moves, a hundred times finer while Shift is held. A
    /// press on the number lets the keyboard type a new one until Enter, Escape, a press elsewhere or a frame without
    /// the field, and the value follows whenever the typed text changes and reads as a number. The value stays with the
    /// game. Returns what the mouse and the keyboard do with the number in this frame, so focused says whether it takes
    /// the typed text.
    Interaction numberField(Context& ui, std::string_view label, float& value, float step = 1.0f);

    /// @brief Shows a whole number like the number field for floats, which dragging changes by one for each pixel.
    Interaction numberField(Context& ui, std::string_view label, int& value);
}
