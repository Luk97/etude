#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <etude/editor/scene/gizmo.h>
#include <etude/editor/scene/gizmo_handle.h>
#include <etude/editor/scene/gizmo_mode.h>
#include <etude/math/vec2.h>
#include <etude/rendering/sprite.h>
#include <etude/scene/components/transform2d.h>

#include <optional>
#include <vector>

using Catch::Matchers::WithinAbs;
using etude::Sprite;
using etude::Transform2D;
using etude::Vec2;
using etude::editor::Gizmo;
using etude::editor::GizmoHandle;
using etude::editor::GizmoMode;

namespace {

    // At zoom 2, the reach of the gizmo of 80 pixels covers 40 units around the entity.
    constexpr float zoom = 2.0f;

    Transform2D at(Vec2 position, float rotation = 0.0f) {
        return {
            .position = position,
            .rotation = rotation,
        };
    }
}

TEST_CASE("The gizmo finds its middle, its axes and its ring under the mouse") {
    Gizmo gizmo;
    const Transform2D transform = at({100.0f, 50.0f});

    // 30 units are 60 pixels along an axis, and one unit beside it is two pixels, well within the tolerance.
    CHECK(gizmo.handleAt(transform, {100.0f, 50.0f}, zoom) == GizmoHandle::Center);
    CHECK(gizmo.handleAt(transform, {130.0f, 51.0f}, zoom) == GizmoHandle::AxisX);
    CHECK(gizmo.handleAt(transform, {101.0f, 80.0f}, zoom) == GizmoHandle::AxisY);
    CHECK(gizmo.handleAt(transform, {130.0f, 80.0f}, zoom) == std::nullopt);

    gizmo.setMode(GizmoMode::Rotate);
    CHECK(gizmo.handleAt(transform, {100.0f, 90.0f}, zoom) == GizmoHandle::Ring);
    CHECK(gizmo.handleAt(transform, {100.0f, 50.0f}, zoom) == std::nullopt);
}

TEST_CASE("The axes of the scale gizmo turn with the entity, those of the move gizmo stay with the world") {
    Gizmo gizmo;
    const Transform2D turned = at({100.0f, 50.0f}, 90.0f);

    // Below the middle lies the y axis of the world, but the x axis of the entity turned by a quarter.
    CHECK(gizmo.handleAt(turned, {100.0f, 80.0f}, zoom) == GizmoHandle::AxisY);
    gizmo.setMode(GizmoMode::Scale);
    CHECK(gizmo.handleAt(turned, {100.0f, 80.0f}, zoom) == GizmoHandle::AxisX);
}

TEST_CASE("Dragging the move gizmo moves the entity along the axis that it holds, and snapping puts it on the grid") {
    Gizmo gizmo;
    gizmo.grab(GizmoHandle::AxisX, at({100.0f, 50.0f}), {130.0f, 50.0f}, zoom);
    REQUIRE(gizmo.grabbed() == GizmoHandle::AxisX);
    CHECK(gizmo.drag({150.0f, 70.0f}, false).position == Vec2{120.0f, 50.0f});
    CHECK(gizmo.drag({150.0f, 70.0f}, true).position == Vec2{128.0f, 50.0f});

    // The square in the middle moves along both axes.
    gizmo.grab(GizmoHandle::Center, at({100.0f, 50.0f}), {100.0f, 50.0f}, zoom);
    CHECK(gizmo.drag({90.0f, 70.0f}, false).position == Vec2{90.0f, 70.0f});
    gizmo.release();
    CHECK(gizmo.grabbed() == std::nullopt);
}

TEST_CASE("Dragging the ring turns the entity by the angle that the mouse goes around it") {
    Gizmo gizmo;
    gizmo.setMode(GizmoMode::Rotate);

    // From the right of the middle to below it is a quarter turn clockwise, since y points down.
    gizmo.grab(GizmoHandle::Ring, at({100.0f, 50.0f}, 10.0f), {140.0f, 50.0f}, zoom);
    CHECK_THAT(gizmo.drag({100.0f, 90.0f}, false).rotation, WithinAbs(100.0, 1e-4));
    CHECK_THAT(gizmo.drag({105.567f, 89.611f}, true).rotation, WithinAbs(90.0, 1e-4));

    // Up from the left of the middle is a small turn clockwise, where the angle jumps from 180 to -180 degrees.
    gizmo.grab(GizmoHandle::Ring, at({100.0f, 50.0f}, 10.0f), {60.0f, 50.0f}, zoom);
    CHECK_THAT(gizmo.drag({60.0f, 40.0f}, false).rotation, WithinAbs(24.036, 1e-3));
}

TEST_CASE("Dragging the scale gizmo stretches the entity by the distance of the mouse from its middle") {
    Gizmo gizmo;
    gizmo.setMode(GizmoMode::Scale);
    Transform2D transform = at({100.0f, 50.0f});
    transform.scale = {2.0f, 1.125f};

    // The grab lies 32 units right of the middle, the mouse then 48 or 37 units. The scale of y stays, also when
    // snapping, though it lies between two steps.
    gizmo.grab(GizmoHandle::AxisX, transform, {132.0f, 50.0f}, zoom);
    CHECK(gizmo.drag({148.0f, 55.0f}, false).scale == Vec2{3.0f, 1.125f});
    CHECK(gizmo.drag({137.0f, 55.0f}, true).scale == Vec2{2.25f, 1.125f});

    // The square stretches evenly: 20 units to the right are 40 pixels, half the reach.
    gizmo.grab(GizmoHandle::Center, transform, {100.0f, 50.0f}, zoom);
    CHECK(gizmo.drag({120.0f, 50.0f}, false).scale == Vec2{3.0f, 1.6875f});
}

TEST_CASE("The gizmo keeps its size on the screen and lights up the given part") {
    Gizmo gizmo;
    const Transform2D transform = at({100.0f, 50.0f});

    // Each axis is a line and a knob, then the square in the middle. The line of the x axis reaches 80 pixels.
    const std::vector<Sprite> lit = gizmo.sprites(transform, zoom, GizmoHandle::AxisX);
    const std::vector<Sprite> unlit = gizmo.sprites(transform, zoom, std::nullopt);
    REQUIRE(lit.size() == 5);
    CHECK_THAT(lit[0].size.x, WithinAbs(40.0, 1e-5));
    CHECK(unlit[0].color.g != unlit[2].color.g);
    CHECK(lit[0].color.g != unlit[0].color.g);
    CHECK(lit[2].color.g == unlit[2].color.g);

    gizmo.setMode(GizmoMode::Rotate);
    CHECK(gizmo.sprites(transform, zoom, std::nullopt).size() == Gizmo::ringSegments);
}
