#pragma once

#include <etude/ui/interaction.h>

#include <string_view>

namespace etude::ui {

    class Context;

    /// @brief Shows a square that a click turns on and off, with the visible text of the label beside it, which takes
    /// clicks as well. The value stays with the game. Returns what the mouse does with the square and its text in this
    /// frame, so clicked says whether the value flipped.
    Interaction checkbox(Context& ui, std::string_view label, bool& value);
}
