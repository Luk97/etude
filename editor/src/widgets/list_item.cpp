#include <etude/editor/widgets/list_item.h>

#include <etude/ui/box.h>
#include <etude/ui/label.h>
#include <etude/ui/modifier.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>

namespace etude::editor {

    ui::Interaction listItem(ui::Context& ui, std::string_view label, bool selected) {
        // The line has a background of its own even when it is not selected, so that it lights up under the mouse.
        const ui::Modifier line = ui::Modifier()
                                      .key(label)
                                      .fillMaxWidth()
                                      .background(selected ? ui::theme::accentColor : ui::theme::panelColor)
                                      .clickable();
        return ui::box(ui, line, [&] { ui::text(ui, ui::visibleText(label)); });
    }
}
