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
