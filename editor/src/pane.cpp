#include <etude/editor/pane.h>

#include <etude/ui/box.h>
#include <etude/ui/column.h>
#include <etude/ui/label.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>

namespace etude::editor {

    namespace {

        /// @brief The space around the content, in pixels at scale 1.
        constexpr float contentPadding = 4.0f;
    }

    ui::Interaction pane(ui::Context& ui, std::string_view title, const ui::Modifier& modifier, ui::Content content) {
        // The pane is a column named by its title, which the modifier of the caller sizes, with the bar above the
        // content.
        ui::Interaction body;
        ui::column(ui, modifier.key(title), [&] {
            // The bar is a line high and spans the whole width, with the visible title at its left.
            const ui::Modifier bar =
                ui::Modifier().fillMaxWidth().height(ui::theme::lineHeight).background(ui::theme::titleColor);
            ui::box(ui, bar, [&] { ui::text(ui, ui::visibleText(title)); });

            // The content takes all the height below the bar and scrolls when it is longer.
            const ui::Modifier area = ui::Modifier()
                                          .key("##content")
                                          .fillMaxWidth()
                                          .weight()
                                          .padding(contentPadding)
                                          .spacedBy(ui::theme::spacing)
                                          .clip()
                                          .verticalScroll();
            body = ui::column(ui, area, content);
        });
        return body;
    }
}
