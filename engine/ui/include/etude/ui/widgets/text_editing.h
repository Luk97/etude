#pragma once

#include <etude/platform/input.h>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace etude::ui {

    class Context;

    /// @brief Applies the typed text and the editing keys of the frame to the text at the cursor, as a field with the
    /// focus does: typed text goes in at the cursor, Backspace and Delete remove characters, the left and right arrow
    /// keys, Home and End move the cursor, and held keys repeat. The cursor counts bytes and jumps over whole UTF-8
    /// characters. Returns whether the text changed.
    bool editText(const Input& input, std::string& text, std::size_t& cursor);

    /// @brief Adds the text to the layout that is open, split at the cursor by a line one pixel wide at scale 1, or
    /// without a line if there is no cursor. The parts leave out the padding of a line, so that they touch the cursor.
    void textWithCursor(Context& ui, std::string_view text, std::optional<std::size_t> cursor);
}
