#include <catch2/catch_test_macros.hpp>

#include <etude/core/log.h>
#include <etude/editor/console/console_pane.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>
#include <etude/ui/theme.h>

#include <algorithm>
#include <cstddef>
#include <format>
#include <vector>

using etude::LogLevel;
using etude::editor::ConsolePane;

TEST_CASE("The console pane keeps the messages with their levels and lets the oldest go once it is full") {
    ConsolePane console;
    console.add(LogLevel::Warning, "first");
    for (std::size_t i = 1; i < ConsolePane::capacity; ++i) {
        console.add(LogLevel::Info, std::format("message {}", i));
    }
    REQUIRE(console.lines().size() == ConsolePane::capacity);
    CHECK(console.lines().front().text == "first");
    CHECK(console.lines().front().level == LogLevel::Warning);

    console.add(LogLevel::Error, "newest");
    CHECK(console.lines().size() == ConsolePane::capacity);
    CHECK(console.lines().front().text == "message 1");
    CHECK(console.lines().back().text == "newest");
    CHECK(console.lines().back().level == LogLevel::Error);
}

TEST_CASE("The console pane shows the newest message first, in the color of its level") {
    constexpr auto fontTexture = static_cast<etude::TextureId>(7);
    etude::Input input;
    etude::ui::Context ui{etude::PixelFont{fontTexture}};
    ConsolePane console;
    console.add(LogLevel::Info, "old");
    console.add(LogLevel::Error, "new");
    ui.beginFrame(input, {640, 360}, 1);
    console.show(ui, etude::ui::Modifier().size(200.0f, 100.0f));
    ui.endFrame();

    // The glyphs of the title come first, then those of the newest line.
    std::vector<float> greens;
    for (const etude::Sprite& sprite : ui.sprites()) {
        if (sprite.texture == fontTexture) {
            greens.push_back(sprite.color.g);
        }
    }
    const auto firstError = std::ranges::find_if(greens, [](float green) { return green < 0.5f; });
    REQUIRE(firstError != greens.end());
    const auto distance = static_cast<std::size_t>(firstError - greens.begin());
    CHECK(distance == 7);
    CHECK(greens.back() == etude::ui::theme::textColor.g);
}
