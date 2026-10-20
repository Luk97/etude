#include <etude/ui/widgets/text_field.h>

#include <etude/ui/context.h>
#include <etude/ui/label.h>
#include <etude/ui/modifier.h>
#include <etude/ui/row.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>
#include <etude/ui/widgets/text_editing.h>

#include <optional>

namespace etude::ui {

    namespace {

        /// @brief The width of the field, in pixels at scale 1.
        constexpr float fieldWidth = 120.0f;
    }

    Interaction textField(Context& ui, std::string_view label, std::string& value) {
        Interaction interaction;

        // The row takes no clicks, but its key keeps the fields of two text fields apart. A label that shows nothing
        // adds no text, so that the field starts right at the left.
        row(ui, Modifier().key(label), [&] {
            if (const std::string_view shown = visibleText(label); !shown.empty()) {
                text(ui, shown);
            }

            // The field takes the focus with a click and cuts off what does not fit.
            const Modifier field = Modifier()
                                       .key("##field")
                                       .width(fieldWidth)
                                       .padding(theme::linePadding)
                                       .background(theme::fieldColor)
                                       .clip()
                                       .focusable();
            interaction = row(ui, field, [&](const Interaction& self) {
                // While the field has the focus, the keys of the frame edit the text before it shows with the cursor.
                if (self.focused) {
                    editText(ui.input(), value, ui.textCursor());
                }
                textWithCursor(ui, value, self.focused ? std::optional(ui.textCursor()) : std::nullopt);
            });
        });
        return interaction;
    }
}
