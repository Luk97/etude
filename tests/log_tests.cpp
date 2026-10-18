#include <catch2/catch_test_macros.hpp>

#include <etude/core/log.h>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

using etude::LogLevel;

TEST_CASE("The log hands each message with its level to the listener, until the listener is taken away") {
    std::vector<std::pair<LogLevel, std::string>> heard;
    etude::setLogListener([&](LogLevel level, std::string_view message) { heard.emplace_back(level, message); });
    etude::logWarning("{} bunnies left", 3);
    etude::logError("no texture");
    etude::setLogListener({});
    etude::logInfo("unheard");

    const std::vector<std::pair<LogLevel, std::string>> expected{
        {LogLevel::Warning, "3 bunnies left"},
        {LogLevel::Error, "no texture"},
    };
    CHECK(heard == expected);
}
