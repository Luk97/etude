#include <etude/ui/widgets/checkbox.h>

#include <etude/ui/box.h>
#include <etude/ui/label.h>
#include <etude/ui/modifier.h>
#include <etude/ui/row.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>

namespace etude::ui {

    Interaction checkbox(Context& ui, std::string_view label, bool& value) {
        // The whole row takes the clicks, so the text beside the square flips the value as well.
        const Modifier entry = Modifier().key(label).clickable();
        return row(ui, entry, [&](const Interaction& self) {
            // The click flips the value before the square shows it, so the square is right in the same frame.
            if (self.clicked) {
                value = !value;
            }

            // The square takes no clicks of its own, so it lights up with the row by hand.
            const Color color = value ? theme::accentColor : theme::buttonColor;
            const Modifier square = Modifier()
                                        .size(theme::lineHeight, theme::lineHeight)
                                        .background(theme::highlight(color, self.hovered, self.held));
            box(ui, square);
            text(ui, visibleText(label));
        });
    }
}
