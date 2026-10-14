#include <etude/ui/widgets/text_field.h>

#include <etude/core/utf8.h>
#include <etude/platform/input.h>
#include <etude/render2d/pixel_font.h>
#include <etude/ui/box.h>
#include <etude/ui/context.h>
#include <etude/ui/label.h>
#include <etude/ui/modifier.h>
#include <etude/ui/row.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>

#include <algorithm>
#include <cstddef>

namespace etude::ui {

    namespace {

        /// @brief The width of the field, in pixels at scale 1.
        constexpr float fieldWidth = 120.0f;

        /// @brief Applies the typed text and the editing keys of the frame to the text at the cursor. The cursor counts
        /// bytes and jumps over whole UTF-8 characters.
        void edit(const Input& input, std::string& text, std::size_t& cursor) {
            // The cursor may stand past the end, after the field gained the focus or the game shortened the text, and
            // inside a character, after the game changed the text. It moves back to the end or to the start of that
            // character.
            cursor = std::min(cursor, text.size());
            if (cursor < text.size()) {
                cursor = previousCharacter(text, cursor + 1);
            }

            // Typed text goes in at the cursor, which moves behind it.
            if (const std::string_view typed = input.text(); !typed.empty()) {
                text.insert(cursor, typed);
                cursor += typed.size();
            }

            // Backspace and Delete remove the whole character before or after the cursor.
            if (input.repeated(Key::Backspace) && cursor > 0) {
                const std::size_t start = previousCharacter(text, cursor);
                text.erase(start, cursor - start);
                cursor = start;
            }
            if (input.repeated(Key::Delete) && cursor < text.size()) {
                text.erase(cursor, nextCharacter(text, cursor) - cursor);
            }

            // The arrow keys move over whole characters, Home and End jump to the start and the end.
            if (input.repeated(Key::Left) && cursor > 0) {
                cursor = previousCharacter(text, cursor);
            }
            if (input.repeated(Key::Right) && cursor < text.size()) {
                cursor = nextCharacter(text, cursor);
            }
            if (input.pressed(Key::Home)) {
                cursor = 0;
            }
            if (input.pressed(Key::End)) {
                cursor = text.size();
            }
        }
    }

    Interaction textField(Context& ui, std::string_view label, std::string& value) {
        Interaction interaction;

        // The row takes no clicks, but its key keeps the fields of two text fields apart.
        row(ui, Modifier().key(label), [&] {
            text(ui, visibleText(label));

            // The field takes the focus with a click and cuts off what does not fit.
            const Modifier field = Modifier()
                                       .key("##field")
                                       .width(fieldWidth)
                                       .padding(theme::linePadding)
                                       .background(theme::fieldColor)
                                       .clip()
                                       .focusable();
            interaction = row(ui, field, [&](const Interaction& self) {
                // While the field has the focus, the keys of the frame edit the text before it shows.
                if (self.focused) {
                    edit(ui.input(), value, ui.textCursor());
                }

                // The cursor splits the text in two, with a line one pixel wide at scale 1 between the halves while the
                // field has the focus. The halves leave out the padding of a line, so that they touch the cursor.
                const std::string_view shown = value;
                const std::size_t split = self.focused ? ui.textCursor() : shown.size();
                text(ui, shown.substr(0, split), Modifier().padding(0.0f));
                if (self.focused) {
                    const float cursorHeight = static_cast<float>(PixelFont::cellHeight);
                    box(ui, Modifier().size(1.0f, cursorHeight).background(theme::textColor));
                }
                text(ui, shown.substr(split), Modifier().padding(0.0f));
            });
        });
        return interaction;
    }
}
