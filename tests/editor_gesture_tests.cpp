#include <catch2/catch_test_macros.hpp>

#include <etude/editor/command.h>
#include <etude/editor/gesture.h>
#include <etude/json/json.h>
#include <etude/scene/component_registry.h>
#include <etude/scene/components/transform2d.h>
#include <etude/scene/scene_json.h>
#include <etude/scene/world.h>

#include <optional>

using etude::Entity;
using etude::World;
using etude::editor::Command;
using etude::editor::Gesture;

namespace {

    etude::ComponentRegistry makeRegistry() {
        etude::ComponentRegistry registry;
        etude::addBuiltinComponents(registry);
        return registry;
    }
}

TEST_CASE("A gesture turns the changes of several frames into one command once nothing holds it any more") {
    const etude::ComponentRegistry registry = makeRegistry();
    World world;
    const Entity ball = world.create();
    world.add(ball, etude::Transform2D{});
    const etude::Json::Object start = etude::writeEntity(world, ball, registry);
    Gesture gesture;

    // Two frames of dragging the rotation, with the mouse button down.
    for (const float rotation : {10.0f, 20.0f}) {
        gesture.watch(world, registry, ball);
        world.get<etude::Transform2D>(ball).rotation = rotation;
        CHECK_FALSE(gesture.settle(world, registry, true));
        CHECK(gesture.open());
    }

    // The frame in which the button is up closes the gesture.
    gesture.watch(world, registry, ball);
    const std::optional<Command> command = gesture.settle(world, registry, false);
    REQUIRE(command);
    REQUIRE(command->changes.size() == 1);
    CHECK(command->changes[0].entity == ball);
    CHECK(command->changes[0].before == start);
    CHECK(command->changes[0].after == etude::writeEntity(world, ball, registry));
    CHECK_FALSE(gesture.open());
}

TEST_CASE("A gesture that leaves the entity as it began gives no command") {
    const etude::ComponentRegistry registry = makeRegistry();
    World world;
    const Entity ball = world.create();
    world.add(ball, etude::Transform2D{});
    Gesture gesture;

    // A frame that changes nothing opens no gesture.
    gesture.watch(world, registry, ball);
    CHECK_FALSE(gesture.settle(world, registry, false));
    CHECK_FALSE(gesture.open());

    // Turning the rotation and back again closes the gesture without a command.
    gesture.watch(world, registry, ball);
    world.get<etude::Transform2D>(ball).rotation = 45.0f;
    CHECK_FALSE(gesture.settle(world, registry, true));
    gesture.watch(world, registry, ball);
    world.get<etude::Transform2D>(ball).rotation = 0.0f;
    CHECK_FALSE(gesture.settle(world, registry, false));
    CHECK_FALSE(gesture.open());
}

TEST_CASE("A gesture keeps its entity while it is open and ends without a command when the entity is gone") {
    const etude::ComponentRegistry registry = makeRegistry();
    World world;
    const Entity ball = world.create();
    const Entity wall = world.create();
    world.add(ball, etude::Transform2D{});
    world.add(wall, etude::Transform2D{});
    Gesture gesture;

    gesture.watch(world, registry, ball);
    world.get<etude::Transform2D>(ball).rotation = 45.0f;
    CHECK_FALSE(gesture.settle(world, registry, true));

    // A new selection while the gesture is open does not take it over.
    gesture.watch(world, registry, wall);
    const std::optional<Command> command = gesture.settle(world, registry, false);
    REQUIRE(command);
    CHECK(command->changes[0].entity == ball);

    // When the entity of an open gesture is gone, the gesture ends without a command.
    gesture.watch(world, registry, ball);
    world.get<etude::Transform2D>(ball).rotation = 90.0f;
    CHECK_FALSE(gesture.settle(world, registry, true));
    world.destroy(ball);
    gesture.watch(world, registry, std::nullopt);
    CHECK_FALSE(gesture.settle(world, registry, false));
    CHECK_FALSE(gesture.open());
}
