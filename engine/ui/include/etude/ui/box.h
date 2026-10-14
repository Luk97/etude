#pragma once

#include <etude/ui/content.h>
#include <etude/ui/interaction.h>
#include <etude/ui/modifier.h>

namespace etude::ui {

    class Context;

    /// @brief Adds a box, a layout whose children lie on top of each other, the one added last on top, each aligned in
    /// the box as the modifier says. Returns what the mouse and the keyboard do with the box in this frame.
    ///
    /// Without a size of its own, the box is as large as its largest child plus its padding. A box without content is a
    /// plain rectangle, such as a colored square or an empty space.
    Interaction box(Context& ui, const Modifier& modifier = {}, Content content = {});

    /// @brief Adds a box without settings of its own around the content.
    Interaction box(Context& ui, Content content);
}
