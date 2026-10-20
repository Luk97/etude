#pragma once

#include <etude/editor/console.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>

namespace etude::editor {

    /// @brief Shows the pane Konsole with the messages that the console keeps, the newest at the top, warnings and
    /// errors in colors of their own. The modifier decides the size and the background of the pane.
    void consolePane(ui::Context& ui, const ui::Modifier& modifier, const Console& console);
}
