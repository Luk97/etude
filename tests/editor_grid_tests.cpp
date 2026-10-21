#include <catch2/catch_test_macros.hpp>

#include <etude/editor/editor_camera.h>
#include <etude/editor/grid.h>
#include <etude/math/vec2.h>
#include <etude/rendering/sprite.h>

#include <algorithm>
#include <vector>

using etude::Sprite;
using etude::Vec2;
using etude::editor::EditorCamera;
using etude::editor::gridLines;

TEST_CASE("The grid draws a line every 32 units across what the camera shows") {
    const EditorCamera camera;

    // The viewport of 128 by 64 pixels shows x from -64 to 64 and y from -32 to 32. Like the right and the bottom edge
    // of a rectangle, x 64 and y 32 lie just outside.
    const std::vector<Sprite> lines = gridLines(camera, {128.0f, 64.0f});
    REQUIRE(lines.size() == 6);
    CHECK(lines[0].position == Vec2{-64.5f, -32.0f});
    CHECK(lines[3].position == Vec2{31.5f, -32.0f});
    CHECK(lines[3].size == Vec2{1.0f, 64.0f});
    CHECK(lines[4].position == Vec2{-64.0f, -32.5f});
    CHECK(lines[5].size == Vec2{128.0f, 1.0f});
}

TEST_CASE("The grid spreads its lines out when the camera zooms out, so that they stay apart on the screen") {
    const EditorCamera camera{
        .zoom = 0.25f,
    };

    // At zoom 1/4, lines 32 units apart would lie 8 pixels apart, so they lie 64 units and 16 pixels apart. A line is
    // still one pixel wide, which takes four units.
    const std::vector<Sprite> lines = gridLines(camera, {64.0f, 16.0f});
    REQUIRE(lines.size() == 5);
    CHECK(lines[0].position == Vec2{-130.0f, -32.0f});
    CHECK(lines[0].size == Vec2{4.0f, 64.0f});
    CHECK(lines[3].position == Vec2{62.0f, -32.0f});
}

TEST_CASE("The lines of the grid through the origin are lighter than the others") {
    const std::vector<Sprite> lines = gridLines(EditorCamera{}, {128.0f, 64.0f});
    const auto isAxis = [](const Sprite& line) { return line.position.x == -0.5f || line.position.y == -0.5f; };
    REQUIRE(std::ranges::count_if(lines, isAxis) == 2);
    for (const Sprite& line : lines) {
        if (!isAxis(line)) {
            CHECK(line.color.r < lines[2].color.r);
        }
    }
}
