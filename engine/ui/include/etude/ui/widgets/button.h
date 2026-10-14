#pragma once

#include <etude/ui/interaction.h>

#include <string_view>

namespace etude::ui {

    class Context;

    /// @brief Shows a button with the visible text of the label. Returns what the mouse does with the button in this
    /// frame, so clicked says whether it was clicked. The whole label names the button, so "OK##2" shows as OK but is a
    /// button of its own.
    Interaction button(Context& ui, std::string_view label);
}
