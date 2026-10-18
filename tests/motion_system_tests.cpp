#include <catch2/catch_test_macros.hpp>

#include <etude/runtime/motion_system.h>
#include <etude/scene/components/transform2d.h>
#include <etude/scene/components/velocity.h>
#include <etude/scene/world.h>

#include <chrono>

using etude::Entity;
using etude::Vec2;
using etude::World;

TEST_CASE("moveScene moves each entity with a Velocity by its velocity over the time and leaves the others") {
    World world;
    const Entity ball = world.create();
    const etude::Transform2D start{
        .position = {100.0f, 50.0f},
        .rotation = 10.0f,
    };
    const etude::Velocity velocity{
        .linear = {300.0f, -100.0f},
        .angular = 90.0f,
    };
    world.add(ball, start);
    world.add(ball, velocity);
    const Entity wall = world.create();
    world.add(wall, start);

    etude::moveScene(world, std::chrono::milliseconds(500));
    CHECK(world.get<etude::Transform2D>(ball).position == Vec2{250.0f, 0.0f});
    CHECK(world.get<etude::Transform2D>(ball).rotation == 55.0f);
    CHECK(world.get<etude::Transform2D>(wall).position == start.position);
    CHECK(world.get<etude::Transform2D>(wall).rotation == start.rotation);
}
