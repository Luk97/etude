#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <etude/math/mat3.h>

#include <numbers>

using etude::Mat3;
using etude::Rect;
using etude::Vec2;

namespace {

    /// @brief Compares both components with a tolerance, because rotations and divisions are not exact in floats.
    void checkNear(Vec2 actual, Vec2 expected) {
        CHECK_THAT(actual.x, Catch::Matchers::WithinAbs(expected.x, 1e-5));
        CHECK_THAT(actual.y, Catch::Matchers::WithinAbs(expected.y, 1e-5));
    }
}

TEST_CASE("Mat3 starts as the identity") {
    CHECK(Mat3{} * Vec2{3.0f, 4.0f} == Vec2{3.0f, 4.0f});
}

TEST_CASE("Mat3 translates points") {
    CHECK(Mat3::translation({10.0f, 20.0f}) * Vec2{1.0f, 2.0f} == Vec2{11.0f, 22.0f});
}

TEST_CASE("Mat3 scales points away from the origin") {
    CHECK(Mat3::scaling({2.0f, 3.0f}) * Vec2{1.0f, 2.0f} == Vec2{2.0f, 6.0f});
}

TEST_CASE("Mat3 rotates clockwise on the screen, because y points down") {
    checkNear(Mat3::rotation(std::numbers::pi_v<float> / 2.0f) * Vec2{1.0f, 0.0f}, {0.0f, 1.0f});
}

TEST_CASE("Mat3 applies the right transform of a product first") {
    const Mat3 scaleThenMove = Mat3::translation({10.0f, 0.0f}) * Mat3::scaling({2.0f, 2.0f});
    CHECK(scaleThenMove * Vec2{1.0f, 1.0f} == Vec2{12.0f, 2.0f});
}

TEST_CASE("Mat3 maps the view rectangle onto clip space") {
    const Rect view{
        .position = {100.0f, 50.0f},
        .size = {800.0f, 600.0f},
    };
    const Mat3 projection = Mat3::orthographic(view);
    checkNear(projection * Vec2{100.0f, 50.0f}, {-1.0f, -1.0f});
    checkNear(projection * Vec2{900.0f, 650.0f}, {1.0f, 1.0f});
    checkNear(projection * Vec2{500.0f, 350.0f}, {0.0f, 0.0f});
}

TEST_CASE("Mat3 transforms work at compile time") {
    STATIC_REQUIRE(Mat3::translation({1.0f, 2.0f}) * Vec2{3.0f, 4.0f} == Vec2{4.0f, 6.0f});
}
