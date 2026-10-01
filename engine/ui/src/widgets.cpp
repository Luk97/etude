#include <etude/ui/context.h>

#include <etude/core/assert.h>
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

        constexpr Color panelColor{
            .r = 0.08f,
            .g = 0.08f,
            .b = 0.11f,
        };

        constexpr Color titleColor{
            .r = 0.2f,
            .g = 0.3f,
            .b = 0.5f,
        };

        /// @brief The padding above and below the text of every widget, so that all widgets are equally high.
        constexpr float linePadding = 3.0f;

        /// @brief The height of a widget, the text with its padding, in pixels at scale 1.
        constexpr float lineHeight = PixelFont::cellHeight + 2.0f * linePadding;

        /// @brief The space between the widgets of a panel, a row or a column, and around the content of a panel.
        constexpr float widgetGap = 3.0f;
        constexpr float panelPadding = 4.0f;

        /// @brief How far one notch of the wheel scrolls, in pixels at scale 1.
        constexpr float scrollStep = 24.0f;

        constexpr float trackWidth = 100.0f;
        constexpr float fieldWidth = 120.0f;
    }

    Scope Context::panel(const PanelSpec& spec) {
        ETUDE_ASSERT(openNodes.size() == 1);
        const Id id = makeId(spec.title, rootId);
        const auto [entry, created] = panels.try_emplace(id);
        PanelState& state = entry->second;
        if (created) {
            state.position = spec.position * static_cast<float>(frameScale);
            panelOrder.push_back(id);
        }
        if (topPanel == id) {
            const float scrolled = state.scroll - frameInput->wheel() * scrollStep * static_cast<float>(frameScale);
            state.scroll = std::clamp(scrolled, 0.0f, state.maxScroll);
        }

        beginBox({
            .label = spec.title,
            .width = SizeRule::pixels(spec.size.x),
            .height = SizeRule::pixels(spec.size.y),
            .background = panelColor,
        });
        const std::size_t node = nodes.size() - 1;
        nodes[node].floating = true;
        nodes[node].panel = id;

        // Dragging the title moves the panel in the same frame, since the layout places it only at the end.
        const Signal title = box({
            .label = "##title",
            .text = visibleText(spec.title),
            .width = SizeRule::parentShare(1.0f),
            .height = SizeRule::fitText(linePadding),
            .clickable = true,
            .background = titleColor,
            .textColor = textColor,
        });
        const Vec2 mouse = frameInput->mousePosition();
        if (title.pressed) {
            state.grab = mouse - state.position;
        }
        if (title.held) {
            state.position = mouse - state.grab;
        }
        nodes[node].offset = state.position;

        beginBox({
            .label = "##content",
            .width = SizeRule::parentShare(1.0f),
            .height = SizeRule::pixels(spec.size.y - lineHeight),
            .padding = panelPadding,
            .gap = widgetGap,
            .clip = true,
        });
        nodes.back().scroll = state.scroll;
        framePanels.push_back({
            .id = id,
            .node = node,
            .content = nodes.size() - 1,
        });
        return Scope(*this, 2);
    }

    Scope Context::row() {
        beginBox({
            .childAxis = Axis::X,
            .gap = widgetGap,
        });
        return Scope(*this, 1);
    }

    Scope Context::column() {
        beginBox({
            .gap = widgetGap,
        });
        return Scope(*this, 1);
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
            .childAxis = Axis::X,
            .padding = linePadding,
            .clip = true,
            .clickable = true,
            .background = fieldColor,
        });
        const Id id = nodes.back().id;
        if (field.pressed && focus != id) {
            focus = id;
            cursor = text.size();
        }
        const bool changed = focus == id && edit(text);

        // While the field has the focus, the cursor splits the text in two.
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
