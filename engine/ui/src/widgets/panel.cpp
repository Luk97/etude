#include <etude/ui/widgets/panel.h>

#include <etude/ui/alignment.h>
#include <etude/ui/box.h>
#include <etude/ui/column.h>
#include <etude/ui/label.h>
#include <etude/ui/modifier.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>

namespace etude::ui {

    namespace {

        /// @brief The space around the content, in pixels at scale 1.
        constexpr float contentPadding = 4.0f;
    }

    Interaction panel(Context& ui, std::string_view title, Vec2 position, Vec2 size, Content content) {
        Interaction interaction;

        // The panel is a floating column of a fixed size, with its title above the content.
        const Modifier window =
            Modifier().key(title).floating(position).size(size.x, size.y).background(theme::panelColor);
        column(ui, window, [&] {
            // The title stretches over the whole width, shows the visible title in its middle and drags the panel.
            const Modifier bar = Modifier()
                                     .key("##title")
                                     .fillMaxWidth()
                                     .background(theme::titleColor)
                                     .contentAlignment(Alignment::Center)
                                     .dragHandle();
            interaction = box(ui, bar, [&] { text(ui, visibleText(title)); });

            // The content fills the rest below the title, which is a line high, and scrolls when it is longer.
            const Modifier body = Modifier()
                                      .key("##content")
                                      .fillMaxWidth()
                                      .height(size.y - theme::lineHeight)
                                      .padding(contentPadding)
                                      .spacedBy(theme::spacing)
                                      .clip()
                                      .verticalScroll();
            column(ui, body, content);
        });
        return interaction;
    }
}
