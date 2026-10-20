#include <catch2/catch_test_macros.hpp>

#include <etude/core/log.h>
#include <etude/editor/console.h>

#include <cstddef>
#include <format>

using etude::LogLevel;
using etude::editor::Console;

TEST_CASE("The console keeps the messages with their levels and lets the oldest go once it is full") {
    Console console;
    console.add(LogLevel::Warning, "first");
    for (std::size_t i = 1; i < Console::capacity; ++i) {
        console.add(LogLevel::Info, std::format("message {}", i));
    }
    REQUIRE(console.lines().size() == Console::capacity);
    CHECK(console.lines().front().text == "first");
    CHECK(console.lines().front().level == LogLevel::Warning);

    console.add(LogLevel::Error, "newest");
    CHECK(console.lines().size() == Console::capacity);
    CHECK(console.lines().front().text == "message 1");
    CHECK(console.lines().back().text == "newest");
    CHECK(console.lines().back().level == LogLevel::Error);
}
