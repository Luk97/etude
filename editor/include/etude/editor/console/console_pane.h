#pragma once

#include <etude/core/log.h>
#include <etude/editor/console/console_line.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>

#include <cstddef>
#include <deque>
#include <string_view>

namespace etude::editor {

    /// @brief The pane Konsole, which keeps the latest messages of the log and shows them, the newest at the top,
    /// warnings and errors in colors of their own. Once it holds as many as it can keep, each new message pushes out
    /// the oldest.
    class ConsolePane {
    public:
        /// @brief How many messages the pane keeps.
        static constexpr std::size_t capacity = 200;

        /// @brief Keeps a copy of the message, as a log listener receives it.
        void add(LogLevel level, std::string_view message);

        /// @brief Returns the messages that the pane keeps, the oldest first.
        const std::deque<ConsoleLine>& lines() const;

        /// @brief Shows the pane. The modifier decides its size and its background.
        void show(ui::Context& ui, const ui::Modifier& modifier) const;

    private:
        std::deque<ConsoleLine> kept;
    };
}
