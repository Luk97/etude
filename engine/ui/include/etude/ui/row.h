#pragma once

#include <etude/ui/content.h>
#include <etude/ui/interaction.h>
#include <etude/ui/modifier.h>

namespace etude::ui {

    class Context;

    /// @brief Adds a row, a layout whose children follow each other from left to right, spaced as the modifier says,
    /// and sit at its top, in its middle or at its bottom as its alignment says. Returns what the mouse and the
    /// keyboard do with the row in this frame.
    ///
    /// Without a size of its own, the row is as wide as its children and their spacing together and as high as its
    /// highest child, plus its padding.
    Interaction row(Context& ui, const Modifier& modifier = {}, Content content = {});

    /// @brief Adds a row without settings of its own around the content.
    Interaction row(Context& ui, Content content);
}
