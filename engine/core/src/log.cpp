#include <etude/core/log.h>

#include <cstdio>
#include <print>
#include <utility>

namespace etude {

    namespace {

        /// @brief Returns the listener, which starts out empty. As a static inside a function it exists from its first
        /// use on, even if a message comes while the program is still starting.
        LogListener& currentListener() {
            static LogListener listener;
            return listener;
        }
    }

    void setLogListener(LogListener listener) {
        currentListener() = std::move(listener);
    }

    void writeLog(LogLevel level, std::string_view message) {
        std::println(stderr, "[{}] {}", toString(level), message);
        if (const LogListener& listener = currentListener()) {
            listener(level, message);
        }
    }
}
