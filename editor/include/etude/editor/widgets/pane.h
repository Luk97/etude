#pragma once

#include <etude/ui/content.h>
#include <etude/ui/context.h>
#include <etude/ui/interaction.h>
#include <etude/ui/modifier.h>

#include <string_view>

namespace etude::editor {

    /// @brief Shows a pane, one of the fixed parts of the editor window: a column with the title as a bar at its top
    /// and the content below, which fills the rest, lines up from top to bottom, is cut off at the edges and scrolls
    /// when it is longer. Unlike a panel of etude.ui, a pane does not float, so the modifier decides its size and, if
    /// it is to be opaque, its background. The title names the pane, so it has to be unique among its siblings. Returns
    /// what the mouse does with the content in this frame, whose rectangle is the space inside the pane.
    ui::Interaction pane(ui::Context& ui, std::string_view title, const ui::Modifier& modifier, ui::Content content);
}
