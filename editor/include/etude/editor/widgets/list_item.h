#pragma once

#include <etude/ui/context.h>
#include <etude/ui/interaction.h>

#include <string_view>

namespace etude::editor {

    /// @brief Shows one line of a list that can be picked by a click, such as an entity in the hierarchy. It spans the
    /// width of the list, shows the label up to ## at its left and has the accent color while it is selected. The whole
    /// label names the line, so lines with the same text need different endings after ##. Returns what the mouse does
    /// with the line in this frame.
    ui::Interaction listItem(ui::Context& ui, std::string_view label, bool selected);
}
