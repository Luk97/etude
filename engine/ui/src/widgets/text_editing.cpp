#include <etude/ui/widgets/text_editing.h>

#include <etude/core/utf8.h>
#include <etude/render2d/pixel_font.h>
#include <etude/ui/box.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>

#include <algorithm>

namespace etude::ui {

    bool editText(const Input& input, std::string& text, std::size_t& cursor) {
        // The cursor may stand past the end, after the field gained the focus or the game shortened the text, and
        // inside a character, after the game changed the text. It moves back to the end or to the start of that
        // character.
        cursor = std::min(cursor, text.size());
        if (cursor < text.size()) {
            cursor = previousCharacter(text, cursor + 1);
        }

        // Typed text goes in at the cursor, which moves behind it.
        bool changed = false;
        if (const std::string_view typed = input.text(); !typed.empty()) {
            text.insert(cursor, typed);
            cursor += typed.size();
            changed = true;
        }

        // Backspace and Delete remove the whole character before or after the cursor.
        if (input.repeated(Key::Backspace) && cursor > 0) {
            const std::size_t start = previousCharacter(text, cursor);
            text.erase(start, cursor - start);
            cursor = start;
            changed = true;
        }
        if (input.repeated(Key::Delete) && cursor < text.size()) {
            text.erase(cursor, nextCharacter(text, cursor) - cursor);
            changed = true;
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
        return changed;
    }

    void textWithCursor(Context& ui, std::string_view text, std::optional<std::size_t> cursor) {
        const std::size_t split = cursor.value_or(text.size());
        ui::text(ui, text.substr(0, split), Modifier().padding(0.0f));
        if (cursor) {
            const float cursorHeight = static_cast<float>(PixelFont::cellHeight);
            box(ui, Modifier().size(1.0f, cursorHeight).background(theme::textColor));
        }
        ui::text(ui, text.substr(split), Modifier().padding(0.0f));
    }
}
