#include <catch2/catch_test_macros.hpp>

#include <etude/editor/history/edit_history.h>
#include <etude/json/json.h>
#include <etude/math/vec2.h>
#include <etude/scene/component_registry.h>
#include <etude/scene/components/name.h>
#include <etude/scene/components/transform2d.h>
#include <etude/scene/components/velocity.h>
#include <etude/scene/scene_json.h>
#include <etude/scene/world.h>

using etude::Entity;
using etude::World;
using etude::editor::EditHistory;

namespace {

    etude::ComponentRegistry makeRegistry() {
        etude::ComponentRegistry registry;
        etude::addBuiltinComponents(registry);
        return registry;
    }

    // Renames the entity in one frame between watch and settle, as the inspector would after the last key.
    void rename(EditHistory& edits, World& world, Entity entity, const char* name) {
        edits.watch(entity);
        world.get<etude::Name>(entity).value = name;
        edits.settle(false);
    }
}

TEST_CASE("An edit becomes one command once nothing holds it, which undo takes back and redo does again") {
    const etude::ComponentRegistry registry = makeRegistry();
    World world;
    const Entity ball = world.create();
    world.add(ball, etude::Name{"Ball"});
    EditHistory edits{world, registry};

    // Two frames of typing with the focus in the field, then a frame without it.
    edits.watch(ball);
    world.get<etude::Name>(ball).value = "Pu";
    edits.settle(true);
    CHECK(edits.busy());
    edits.watch(ball);
    world.get<etude::Name>(ball).value = "Puck";
    edits.settle(true);
    edits.watch(ball);
    edits.settle(false);
    CHECK_FALSE(edits.busy());

    REQUIRE(edits.undo());
    CHECK(world.get<etude::Name>(ball).value == "Ball");
    REQUIRE(edits.redo());
    CHECK(world.get<etude::Name>(ball).value == "Puck");
}

TEST_CASE("A new command drops what could be redone, and undo and redo report when there is nothing to do") {
    const etude::ComponentRegistry registry = makeRegistry();
    World world;
    const Entity ball = world.create();
    world.add(ball, etude::Name{"Ball"});
    EditHistory edits{world, registry};
    CHECK_FALSE(edits.undo());
    CHECK_FALSE(edits.redo());

    rename(edits, world, ball, "Puck");
    REQUIRE(edits.undo());
    rename(edits, world, ball, "Disc");
    CHECK_FALSE(edits.redo());
    CHECK(world.get<etude::Name>(ball).value == "Disc");
}

TEST_CASE("Creating an entity is a command, which undo takes away and redo brings back under its handle") {
    const etude::ComponentRegistry registry = makeRegistry();
    World world;
    EditHistory edits{world, registry};
    const Entity entity = edits.create({10.0f, 20.0f});
    CHECK(world.get<etude::Name>(entity).value == "Entity");
    CHECK(world.get<etude::Transform2D>(entity).position == etude::Vec2{10.0f, 20.0f});

    REQUIRE(edits.undo());
    CHECK_FALSE(world.alive(entity));
    REQUIRE(edits.redo());
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
    const etude::Json::Object components = etude::writeEntity(world, ball, registry);
    EditHistory edits{world, registry};
    edits.destroy(ball);
    CHECK_FALSE(world.alive(ball));

    REQUIRE(edits.undo());
    REQUIRE(world.alive(ball));
    CHECK(etude::writeEntity(world, ball, registry) == components);
    REQUIRE(edits.redo());
    CHECK_FALSE(world.alive(ball));
}
