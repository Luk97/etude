#pragma once

#include <etude/core/log.h>
#include <etude/editor/console_line.h>

#include <cstddef>
#include <deque>
#include <string_view>

namespace etude::editor {

    /// @brief Keeps the latest messages of the log for the console pane, the oldest first. Once it holds as many as it
    /// can keep, each new message pushes out the oldest.
    class Console {
    public:
        /// @brief How many messages the console keeps.
        static constexpr std::size_t capacity = 200;

        /// @brief Keeps a copy of the message, as a log listener receives it.
        void add(LogLevel level, std::string_view message);

        const std::deque<ConsoleLine>& lines() const;

    private:
        std::deque<ConsoleLine> kept;
    };
}
