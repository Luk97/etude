#pragma once

#include <etude/ui/interaction.h>
#include <etude/ui/modifier.h>

#include <string_view>

namespace etude::ui {

    class Context;

    /// @brief Shows a line of text as it is, ## included, in the text color of the theme and in the middle of its
    /// element. Returns what the mouse does with the text in this frame, if the modifier gives it a key and makes it
    /// clickable.
    ///
    /// Without a padding of its own, the text keeps the padding of a line, so that it lines up with the widgets beside
    /// it. Without a size of its own, the element is as large as the text plus the padding.
    Interaction text(Context& ui, std::string_view line, const Modifier& modifier = {});
}
