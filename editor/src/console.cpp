#include <etude/editor/console.h>

#include <string>

namespace etude::editor {

    void Console::add(LogLevel level, std::string_view message) {
        if (kept.size() == capacity) {
            kept.pop_front();
        }
        kept.push_back({
            .level = level,
            .text = std::string(message),
        });
    }

    const std::deque<ConsoleLine>& Console::lines() const {
        return kept;
    }
}
