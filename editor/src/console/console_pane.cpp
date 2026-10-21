#include <etude/editor/console/console_pane.h>

#include <etude/editor/widgets/pane.h>
#include <etude/math/color.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>

#include <ranges>
#include <string>
#include <utility>

namespace etude::editor {

    namespace {

        /// @brief The color of warnings, a warm yellow.
        constexpr Color warningColor{
            .r = 1.0f,
            .g = 0.8f,
            .b = 0.3f,
        };

        /// @brief The color of errors and fatal errors, a light red.
        constexpr Color errorColor{
            .r = 1.0f,
            .g = 0.4f,
            .b = 0.4f,
        };

        /// @brief Returns the color in which the pane shows a message of the level.
        Color colorOf(LogLevel level) {
            switch (level) {
                case LogLevel::Info:
                    return ui::theme::textColor;
                case LogLevel::Warning:
                    return warningColor;
                case LogLevel::Error:
                case LogLevel::Fatal:
                    return errorColor;
            }
            std::unreachable();
        }
    }

    void ConsolePane::add(LogLevel level, std::string_view message) {
        if (kept.size() == capacity) {
            kept.pop_front();
        }
        kept.push_back({
            .level = level,
            .text = std::string(message),
        });
    }

    const std::deque<ConsoleLine>& ConsolePane::lines() const {
        return kept;
    }

    void ConsolePane::show(ui::Context& ui, const ui::Modifier& modifier) const {
        pane(ui, "Konsole", modifier, [&] {
            for (const ConsoleLine& line : kept | std::views::reverse) {
                ui::text(ui, line.text, ui::Modifier().textColor(colorOf(line.level)));
            }
        });
    }
}
