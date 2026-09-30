#include <etude/ui/context.h>

#include <etude/core/utf8.h>

#include <algorithm>
#include <format>

namespace etude::ui {

    namespace {

        constexpr Color textColor{
            .r = 0.9f,
            .g = 0.9f,
            .b = 0.9f,
        };

        constexpr Color buttonColor{
            .r = 0.25f,
            .g = 0.27f,
            .b = 0.33f,
        };

        constexpr Color accentColor{
            .r = 0.35f,
            .g = 0.55f,
            .b = 0.9f,
        };

        constexpr Color fieldColor{
            .r = 0.12f,
            .g = 0.13f,
            .b = 0.16f,
        };

        /// @brief The padding above and below the text of every widget, so that all widgets are equally high.
        constexpr float linePadding = 3.0f;

        /// @brief The height of a widget, the text with its padding, in pixels at scale 1.
        constexpr float lineHeight = PixelFont::cellHeight + 2.0f * linePadding;

        constexpr float trackWidth = 100.0f;
        constexpr float fieldWidth = 120.0f;
    }

    void Context::label(std::string_view text) {
        box({
            .text = text,
            .width = SizeRule::fitText(linePadding),
            .height = SizeRule::fitText(linePadding),
            .textColor = textColor,
        });
    }

    bool Context::button(std::string_view label) {
        const Signal signal = box({
            .label = label,
            .width = SizeRule::fitText(6.0f),
            .height = SizeRule::fitText(linePadding),
            .clickable = true,
            .background = buttonColor,
            .textColor = textColor,
        });
        return signal.clicked;
    }

    bool Context::checkbox(std::string_view label, bool& value) {
        // The whole row takes the clicks, so the text beside the square flips the value as well.
        const Signal row = beginBox({
            .label = label,
            .childAxis = Axis::X,
            .clickable = true,
        });
        if (row.clicked) {
            value = !value;
        }
        box({
            .width = SizeRule::pixels(lineHeight),
            .height = SizeRule::pixels(lineHeight),
            .background = highlight(value ? accentColor : buttonColor, row.hovered, row.held),
        });
        box({
            .text = visibleText(label),
            .width = SizeRule::fitText(linePadding),
            .height = SizeRule::fitText(linePadding),
            .textColor = textColor,
        });
        endBox();
        return row.clicked;
    }

    bool Context::slider(std::string_view label, float& value, float min, float max) {
        const float previous = value;
        beginBox({
            .label = label,
            .childAxis = Axis::X,
        });
        box({
            .text = visibleText(label),
            .width = SizeRule::fitText(linePadding),
            .height = SizeRule::fitText(linePadding),
            .textColor = textColor,
        });
        const Signal track = beginBox({
            .label = "##track",
            .width = SizeRule::pixels(trackWidth),
            .height = SizeRule::fitText(linePadding),
            .clickable = true,
            .background = buttonColor,
        });
        if (track.held) {
            const float share = (frameInput->mousePosition().x - track.rect.position.x) / track.rect.size.x;
            value = min + (max - min) * std::clamp(share, 0.0f, 1.0f);
        }
        box({
            .width = SizeRule::parentShare((value - min) / (max - min)),
            .height = SizeRule::parentShare(1.0f),
            .background = accentColor,
        });
        endBox();
        box({
            .text = std::format("{:.2f}", value),
            .width = SizeRule::fitText(linePadding),
            .height = SizeRule::fitText(linePadding),
            .textColor = textColor,
        });
        endBox();
        return value != previous;
    }

    bool Context::textField(std::string_view label, std::string& text) {
        beginBox({
            .label = label,
            .childAxis = Axis::X,
        });
        box({
            .text = visibleText(label),
            .width = SizeRule::fitText(linePadding),
            .height = SizeRule::fitText(linePadding),
            .textColor = textColor,
        });
        const Signal field = beginBox({
            .label = "##field",
            .width = SizeRule::pixels(fieldWidth),
            .height = SizeRule::fitText(linePadding),
            .clickable = true,
            .background = fieldColor,
        });
        const Id id = nodes.back().id;
        if (field.pressed && focus != id) {
            focus = id;
            cursor = text.size();
        }
        const bool changed = focus == id && edit(text);

        // The line of text sits inside the padding, split at the cursor while the field has the focus.
        box({
            .height = SizeRule::pixels(linePadding),
        });
        beginBox({
            .childAxis = Axis::X,
        });
        box({
            .width = SizeRule::pixels(linePadding),
        });
        const std::string_view shown = text;
        const std::size_t split = focus == id ? cursor : shown.size();
        box({
            .text = shown.substr(0, split),
            .width = SizeRule::fitText(0.0f),
            .height = SizeRule::fitText(0.0f),
            .textColor = textColor,
        });
        if (focus == id) {
            box({
                .width = SizeRule::pixels(1.0f),
                .height = SizeRule::fitText(0.0f),
                .background = textColor,
            });
        }
        box({
            .text = shown.substr(split),
            .width = SizeRule::fitText(0.0f),
            .height = SizeRule::fitText(0.0f),
            .textColor = textColor,
        });
        endBox();
        endBox();
        endBox();
        return changed;
    }

    bool Context::edit(std::string& text) {
        // The game may have changed the text since the last frame.
        cursor = std::min(cursor, text.size());

        bool changed = false;
        if (const std::string_view typed = frameInput->text(); !typed.empty()) {
            text.insert(cursor, typed);
            cursor += typed.size();
            changed = true;
        }
        if (frameInput->repeated(Key::Backspace) && cursor > 0) {
            const std::size_t start = previousCharacter(text, cursor);
            text.erase(start, cursor - start);
            cursor = start;
            changed = true;
        }
        if (frameInput->repeated(Key::Delete) && cursor < text.size()) {
            text.erase(cursor, nextCharacter(text, cursor) - cursor);
            changed = true;
        }
        if (frameInput->repeated(Key::Left) && cursor > 0) {
            cursor = previousCharacter(text, cursor);
        }
        if (frameInput->repeated(Key::Right) && cursor < text.size()) {
            cursor = nextCharacter(text, cursor);
        }
        if (frameInput->pressed(Key::Home)) {
            cursor = 0;
        }
        if (frameInput->pressed(Key::End)) {
            cursor = text.size();
        }
        if (frameInput->pressed(Key::Enter) || frameInput->pressed(Key::Escape)) {
            focus = Id{};
        }
        return changed;
    }
}
