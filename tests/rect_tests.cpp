#include <catch2/catch_test_macros.hpp>

#include <etude/math/rect.h>

using etude::Rect;

TEST_CASE("Rect contains its left and top edges, but not its right and bottom ones") {
    const Rect rect{
        .position = {10.0f, 20.0f},
        .size = {30.0f, 40.0f},
    };
    CHECK(rect.contains({10.0f, 20.0f}));
    CHECK(rect.contains({39.0f, 59.0f}));
    CHECK_FALSE(rect.contains({40.0f, 30.0f}));
    CHECK_FALSE(rect.contains({20.0f, 60.0f}));
    CHECK_FALSE(rect.contains({9.0f, 30.0f}));
}

TEST_CASE("Rect intersection keeps the part that both rectangles cover, and no size where they do not meet") {
    const Rect wide{
        .position = {0.0f, 0.0f},
        .size = {100.0f, 50.0f},
    };
    const Rect tall{
        .position = {60.0f, 20.0f},
        .size = {100.0f, 100.0f},
    };
    const Rect apart{
        .position = {200.0f, 200.0f},
        .size = {10.0f, 10.0f},
    };
    const Rect overlap = wide.intersection(tall);
    CHECK(overlap.position == etude::Vec2{60.0f, 20.0f});
    CHECK(overlap.size == etude::Vec2{40.0f, 30.0f});
    CHECK(wide.intersection(apart).size == etude::Vec2{0.0f, 0.0f});
}
