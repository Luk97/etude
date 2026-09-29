#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <etude/math/color.h>

using Catch::Matchers::WithinAbs;

TEST_CASE("toLinear turns sRGB into linear light and keeps alpha") {
    const etude::Color linear = etude::toLinear({0.5f, 0.02f, 1.0f, 0.25f});
    CHECK_THAT(linear.r, WithinAbs(0.214041, 1e-6));
    CHECK_THAT(linear.g, WithinAbs(0.02 / 12.92, 1e-7));
    CHECK(linear.b == 1.0f);
    CHECK(linear.a == 0.25f);
}
