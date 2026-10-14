#pragma once

#include <etude/ui/interaction.h>

#include <string_view>

namespace etude::ui {

    class Context;

    /// @brief Shows a bar that sets the value between min and max to where the mouse presses or drags it, with the
    /// visible text of the label before it and the value after it. The value stays with the game. Returns what the
    /// mouse does with the bar in this frame, so held says whether the value follows the mouse. Min has to be smaller
    /// than max.
    Interaction slider(Context& ui, std::string_view label, float& value, float min, float max);
}
