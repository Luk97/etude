#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <etude/math/vec2.h>

/// @brief Compares both components with a tolerance, because rotations and divisions are not exact in floats.
inline void checkNear(etude::Vec2 actual, etude::Vec2 expected) {
    CHECK_THAT(actual.x, Catch::Matchers::WithinAbs(expected.x, 1e-5));
    CHECK_THAT(actual.y, Catch::Matchers::WithinAbs(expected.y, 1e-5));
}
