#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "check_near.h"

#include <etude/editor/outline.h>
#include <etude/math/color.h>
#include <etude/math/vec2.h>
#include <etude/rendering/sprite.h>

#include <array>
#include <numbers>

using etude::Sprite;
using etude::Vec2;
using etude::editor::outline;

namespace {

    constexpr etude::Color orange{
        .r = 1.0f,
        .g = 0.5f,
    };
}

TEST_CASE("An outline puts four edges of the given thickness on the edges of the rectangle") {
    const std::array<Sprite, 4> edges = outline({100.0f, 50.0f}, {40.0f, 20.0f}, 0.0f, 2.0f, orange);

    // The top and the bottom edge reach one pixel over each corner, the left and the right edge run between them.
    checkNear(edges[0].position, {79.0f, 39.0f});
    CHECK(edges[0].size == Vec2{42.0f, 2.0f});
    checkNear(edges[1].position, {79.0f, 59.0f});
    checkNear(edges[2].position, {79.0f, 40.0f});
    CHECK(edges[2].size == Vec2{2.0f, 20.0f});
    checkNear(edges[3].position, {119.0f, 40.0f});
    CHECK(edges[3].color.g == orange.g);
}

TEST_CASE("An outline turns with its rectangle and frames a mirrored one like an unmirrored one") {
    const std::array<Sprite, 4> edges = outline({0.0f, 0.0f}, {-40.0f, 20.0f}, 90.0f, 2.0f, orange);

    // Turned clockwise by a quarter, the top edge lies right of the center, still 42 by 2 before its own turn, and the
    // left edge above it.
    checkNear(edges[0].position + edges[0].size * 0.5f, {10.0f, 0.0f});
    CHECK(edges[0].size == Vec2{42.0f, 2.0f});
    CHECK_THAT(edges[0].rotation, Catch::Matchers::WithinAbs(std::numbers::pi / 2.0, 1e-6));
    checkNear(edges[2].position + edges[2].size * 0.5f, {0.0f, -20.0f});
}
