#include <catch2/catch_test_macros.hpp>

#include <etude/editor/entity_change.h>
#include <etude/editor/entity_commands.h>
#include <etude/editor/history.h>
#include <etude/json/json.h>
#include <etude/math/vec2.h>
#include <etude/scene/component_registry.h>
#include <etude/scene/components/name.h>
#include <etude/scene/components/transform2d.h>
#include <etude/scene/components/velocity.h>
#include <etude/scene/scene_json.h>
#include <etude/scene/world.h>

#include <utility>

using etude::Entity;
using etude::Json;
using etude::World;
using etude::editor::History;

namespace {

    etude::ComponentRegistry makeRegistry() {
        etude::ComponentRegistry registry;
        etude::addBuiltinComponents(registry);
        return registry;
    }

    // Renames the entity and records it as one command, as the inspector and a gesture would.
    void rename(
        World& world,
        const etude::ComponentRegistry& registry,
        History& history,
        Entity entity,
        const char* name
    ) {
        Json::Object before = etude::writeEntity(world, entity, registry);
        world.get<etude::Name>(entity).value = name;
        etude::editor::EntityChange change{
            .entity = entity,
            .before = std::move(before),
            .after = etude::writeEntity(world, entity, registry),
        };
        history.push({
            .changes = {std::move(change)},
        });
    }
}

TEST_CASE("Undo and redo take back and do again what a command changed, and a new command drops what could be redone") {
    const etude::ComponentRegistry registry = makeRegistry();
    World world;
    const Entity ball = world.create();
    world.add(ball, etude::Name{"Ball"});
    History history;
    rename(world, registry, history, ball, "Puck");

    REQUIRE(history.undo(world, registry));
    CHECK(world.get<etude::Name>(ball).value == "Ball");
    REQUIRE(history.redo(world, registry));
    CHECK(world.get<etude::Name>(ball).value == "Puck");

    REQUIRE(history.undo(world, registry));
    rename(world, registry, history, ball, "Disc");
    CHECK_FALSE(history.redo(world, registry));
    CHECK(world.get<etude::Name>(ball).value == "Disc");
}

TEST_CASE("Undo and redo report when there is nothing to take back or to do again") {
    const etude::ComponentRegistry registry = makeRegistry();
    World world;
    History history;
    CHECK_FALSE(history.undo(world, registry));
    CHECK_FALSE(history.redo(world, registry));
}

TEST_CASE("createEntity records the new entity, which undo takes away and redo brings back under its handle") {
    const etude::ComponentRegistry registry = makeRegistry();
    World world;
    History history;
    const Entity entity = etude::editor::createEntity(world, registry, history, {10.0f, 20.0f});
    CHECK(world.get<etude::Name>(entity).value == "Entity");
    CHECK(world.get<etude::Transform2D>(entity).position == etude::Vec2{10.0f, 20.0f});

    REQUIRE(history.undo(world, registry));
    CHECK_FALSE(world.alive(entity));
    REQUIRE(history.redo(world, registry));
    REQUIRE(world.alive(entity));
    CHECK(world.get<etude::Transform2D>(entity).position == etude::Vec2{10.0f, 20.0f});
}

TEST_CASE("Undo brings a destroyed entity back under its old handle with all its components") {
    const etude::ComponentRegistry registry = makeRegistry();
    World world;
    const Entity ball = world.create();
    world.add(ball, etude::Name{"Ball"});
    world.add(ball, etude::Transform2D{});
    const etude::Velocity velocity{
        .linear = {300.0f, 200.0f},
    };
    world.add(ball, velocity);
    const Json::Object components = etude::writeEntity(world, ball, registry);
    History history;
    etude::editor::destroyEntity(world, registry, history, ball);
    CHECK_FALSE(world.alive(ball));

    REQUIRE(history.undo(world, registry));
    REQUIRE(world.alive(ball));
    CHECK(etude::writeEntity(world, ball, registry) == components);
    REQUIRE(history.redo(world, registry));
    CHECK_FALSE(world.alive(ball));
}
