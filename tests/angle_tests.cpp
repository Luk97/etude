#include <catch2/catch_test_macros.hpp>

#include <etude/math/angle.h>

#include <numbers>

TEST_CASE("toRadians turns degrees into radians, also at compile time") {
    STATIC_REQUIRE(etude::toRadians(0.0f) == 0.0f);
    STATIC_REQUIRE(etude::toRadians(180.0f) == std::numbers::pi_v<float>);
    STATIC_REQUIRE(etude::toRadians(-90.0f) == -std::numbers::pi_v<float> / 2.0f);
}
