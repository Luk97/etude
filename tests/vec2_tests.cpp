#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <etude/core/vec2.h>

using etude::Vec2;

TEST_CASE("Vec2 adds and subtracts component-wise") {
    CHECK(Vec2{1.0f, 2.0f} + Vec2{3.0f, 4.0f} == Vec2{4.0f, 6.0f});
    CHECK(Vec2{3.0f, 4.0f} - Vec2{1.0f, 1.0f} == Vec2{2.0f, 3.0f});
}

TEST_CASE("Vec2 scales by a scalar") {
    CHECK(Vec2{1.0f, 2.0f} * 2.0f == Vec2{2.0f, 4.0f});
}

TEST_CASE("Vec2 computes its length") {
    const Vec2 v{3.0f, 4.0f};
    CHECK(v.lengthSquared() == 25.0f);
    CHECK_THAT(v.length(), Catch::Matchers::WithinAbs(5.0, 1e-6));
}

TEST_CASE("Vec2 operations work at compile time") {
    STATIC_REQUIRE(Vec2{1.0f, 2.0f} + Vec2{3.0f, 4.0f} == Vec2{4.0f, 6.0f});
}
