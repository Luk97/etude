#pragma once

#include <etude/math/vec2.h>
#include <etude/ui/content.h>
#include <etude/ui/interaction.h>

#include <string_view>

namespace etude::ui {

    class Context;

    /// @brief Shows a panel: an element with a title that floats above everything else, with the content below the
    /// title.
    ///
    /// The panel first appears at the position and keeps its size, both in pixels at scale 1. Dragging the title moves
    /// it, a click anywhere on it brings it to the front, and the wheel scrolls the content while the mouse is over it.
    /// The content lines up from top to bottom and is cut off at the edge of the panel. The title names the panel, so
    /// it has to be unique among the panels, and panels are only allowed at the top level, not inside other elements.
    /// Returns what the mouse does with the title in this frame, so held says whether the panel is being dragged.
    Interaction panel(Context& ui, std::string_view title, Vec2 position, Vec2 size, Content content);
}
