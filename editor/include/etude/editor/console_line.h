#pragma once

#include <etude/core/log.h>

#include <string>

namespace etude::editor {

    /// @brief One message of the log as the console keeps it, with its level.
    struct ConsoleLine {
        LogLevel level = LogLevel::Info;
        std::string text;
    };
}
