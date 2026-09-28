#include <catch2/catch_test_macros.hpp>

#include "check_near.h"

#include <etude/render2d/camera2d.h>

using etude::Camera2D;
using etude::Mat3;
using etude::Vec2;

TEST_CASE("Camera2D shows its position in the top left corner of the window") {
    const Camera2D camera{
        .position = {100.0f, 50.0f},
    };
    const Mat3 viewProjection = camera.viewProjection({800, 600});
    checkNear(viewProjection * Vec2{100.0f, 50.0f}, {-1.0f, -1.0f});
    checkNear(viewProjection * Vec2{900.0f, 650.0f}, {1.0f, 1.0f});
}

TEST_CASE("Camera2D shows half as much of the world at zoom 2") {
    const Camera2D camera{
        .zoom = 2.0f,
    };
    checkNear(camera.viewProjection({800, 600}) * Vec2{400.0f, 300.0f}, {1.0f, 1.0f});
}
