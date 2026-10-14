#pragma once

#include <etude/ui/content.h>
#include <etude/ui/interaction.h>
#include <etude/ui/modifier.h>

namespace etude::ui {

    class Context;

    /// @brief Adds a column, a layout whose children follow each other from top to bottom, spaced as the modifier says,
    /// and sit at its left, in its middle or at its right as its alignment says. Returns what the mouse and the
    /// keyboard do with the column in this frame.
    ///
    /// Without a size of its own, the column is as high as its children and their spacing together and as wide as its
    /// widest child, plus its padding.
    Interaction column(Context& ui, const Modifier& modifier = {}, Content content = {});

    /// @brief Adds a column without settings of its own around the content.
    Interaction column(Context& ui, Content content);
}
