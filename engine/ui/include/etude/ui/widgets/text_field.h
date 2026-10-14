#pragma once

#include <etude/ui/interaction.h>

#include <string>
#include <string_view>

namespace etude::ui {

    class Context;

    /// @brief Shows a field that takes typed text from a click on it until Enter, Escape or a click elsewhere, with the
    /// visible text of the label before it. Backspace and Delete remove characters, the arrow keys, Home and End move
    /// the cursor, and held keys repeat. The text stays with the game and has to be valid UTF-8, and the field cuts off
    /// what does not fit. Returns what the mouse and the keyboard do with the field in this frame, so focused says
    /// whether it takes the typed text.
    Interaction textField(Context& ui, std::string_view label, std::string& value);
}
