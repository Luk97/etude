#include <catch2/catch_test_macros.hpp>

#include "check_near.h"

#include <etude/editor/editor_camera.h>
#include <etude/math/vec2.h>
#include <etude/render2d/camera2d.h>

using etude::Vec2;
using etude::editor::EditorCamera;

namespace {

    constexpr Vec2 viewport{400.0f, 300.0f};
}

TEST_CASE("The editor camera shows its center in the middle of the viewport") {
    const EditorCamera camera{
        .center = {100.0f, 50.0f},
        .zoom = 2.0f,
    };

    // At zoom 2, the viewport of 400 by 300 pixels shows 200 by 150 units around the center.
    const etude::Camera2D view = camera.view(viewport);
    CHECK(view.position == Vec2{0.0f, -25.0f});
    CHECK(view.zoom == 2.0f);
    CHECK(camera.toWorld({200.0f, 150.0f}, viewport) == Vec2{100.0f, 50.0f});
    CHECK(camera.toWorld({0.0f, 0.0f}, viewport) == view.position);
}

TEST_CASE("Panning moves the world along with the mouse") {
    EditorCamera camera{
        .zoom = 2.0f,
    };

    // At zoom 2, ten pixels to the right are five units, by which the world follows the mouse and the center goes back.
    camera.pan({10.0f, -4.0f});
    CHECK(camera.center == Vec2{-5.0f, 2.0f});
}

TEST_CASE("Zooming keeps the point of the world under the mouse in its place") {
    EditorCamera camera;
    const Vec2 mouse{300.0f, 100.0f};
    const Vec2 under = camera.toWorld(mouse, viewport);

    camera.zoomAt(1.0f, mouse, viewport);
    CHECK(camera.zoom == EditorCamera::zoomPerNotch);
    checkNear(camera.toWorld(mouse, viewport), under);

    camera.zoomAt(-3.0f, mouse, viewport);
    checkNear(camera.toWorld(mouse, viewport), under);
}

TEST_CASE("The zoom of the editor camera stays within its limits") {
    EditorCamera camera;
    camera.zoomAt(100.0f, {}, viewport);
    CHECK(camera.zoom == EditorCamera::maxZoom);
    camera.zoomAt(-200.0f, {}, viewport);
    CHECK(camera.zoom == EditorCamera::minZoom);
}
