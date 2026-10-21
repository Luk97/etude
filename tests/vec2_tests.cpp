#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "check_near.h"

#include <etude/math/vec2.h>

#include <numbers>

using etude::Vec2;

TEST_CASE("Vec2 adds and subtracts component-wise") {
    CHECK(Vec2{1.0f, 2.0f} + Vec2{3.0f, 4.0f} == Vec2{4.0f, 6.0f});
    CHECK(Vec2{3.0f, 4.0f} - Vec2{1.0f, 1.0f} == Vec2{2.0f, 3.0f});
}

TEST_CASE("Vec2 scales by a scalar and divides by one") {
    CHECK(Vec2{1.0f, 2.0f} * 2.0f == Vec2{2.0f, 4.0f});
    CHECK(Vec2{3.0f, -4.0f} / 2.0f == Vec2{1.5f, -2.0f});
}

TEST_CASE("Vec2 computes its length") {
    const Vec2 v{3.0f, 4.0f};
    CHECK(v.lengthSquared() == 25.0f);
    CHECK_THAT(v.length(), Catch::Matchers::WithinAbs(5.0, 1e-6));
}

TEST_CASE("Vec2 turns clockwise on the screen, where y points down") {
    // A quarter turn takes right to down and down to left.
    const float quarter = std::numbers::pi_v<float> / 2.0f;
    checkNear(Vec2{1.0f, 0.0f}.rotated(quarter), {0.0f, 1.0f});
    checkNear(Vec2{0.0f, 2.0f}.rotated(quarter), {-2.0f, 0.0f});
}

TEST_CASE("Vec2 knows its angle from the x axis, which turning it by an angle increases") {
    // Down is a quarter turn from the x axis, and the diagonal down and to the right turned by a quarter lies at three
    // eighths of a turn.
    const float quarter = std::numbers::pi_v<float> / 2.0f;
    const Vec2 down{0.0f, 2.0f};
    const Vec2 diagonal{1.0f, 1.0f};
    CHECK(Vec2{3.0f, 0.0f}.angle() == 0.0f);
    CHECK_THAT(down.angle(), Catch::Matchers::WithinAbs(quarter, 1e-6));
    CHECK_THAT(diagonal.rotated(quarter).angle(), Catch::Matchers::WithinAbs(1.5f * quarter, 1e-6));
}

TEST_CASE("Vec2 operations work at compile time") {
    STATIC_REQUIRE(Vec2{1.0f, 2.0f} + Vec2{3.0f, 4.0f} == Vec2{4.0f, 6.0f});
    STATIC_REQUIRE(Vec2{4.0f, 6.0f} / 2.0f == Vec2{2.0f, 3.0f});
}
