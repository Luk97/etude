#include <catch2/catch_test_macros.hpp>

#include <etude/json/json.h>
#include <etude/math/vec2.h>
#include <etude/scene/component_registry.h>
#include <etude/scene/components.h>
#include <etude/scene/reflection.h>
#include <etude/scene/scene_json.h>
#include <etude/scene/world.h>

#include <string>
#include <string_view>
#include <tuple>

using etude::Entity;
using etude::Json;
using etude::World;

namespace {

    // Only the tests know this component. It needs nothing but a type name and a field list to be saved and loaded,
    // which is what milestone 4 promises for every new component.
    struct Health {
        int points = 100;
        float regeneration = 0.0f;

        static constexpr std::string_view typeName = "Health";
        static constexpr std::tuple fields{
            etude::Field{"points", &Health::points},
            etude::Field{"regeneration", &Health::regeneration},
        };
    };

    struct Velocity {
        etude::Vec2 value;
    };

    etude::ComponentRegistry makeRegistry() {
        etude::ComponentRegistry registry;
        etude::addBuiltinComponents(registry);
        registry.add<Health>();
        return registry;
    }

    std::string errorOf(std::string_view text) {
        const auto world = etude::readScene(etude::parseJson(text).value(), makeRegistry());
        REQUIRE_FALSE(world);
        return world.error();
    }
}

TEST_CASE("writeScene writes the version and each entity with its components in the order of the registry") {
    World world;
    const Entity player = world.create();
    const etude::Transform2D transform{
        .position = {640.0f, 360.0f},
    };
    world.add(player, transform);
    world.add(player, etude::Name{"Player"});
    world.create();

    CHECK(etude::writeJson(etude::writeScene(world, makeRegistry())) == R"({
    "version": 1,
    "entities": [
        {
            "Name": {
                "value": "Player"
            },
            "Transform2D": {
                "position": [640, 360],
                "rotation": 0,
                "scale": [1, 1]
            }
        },
        {}
    ]
})");
}

TEST_CASE("writeScene leaves out components whose type the registry does not know") {
    World world;
    world.add(world.create(), Velocity{});
    const Json expected = Json::Object{
        {"version", 1},
        {"entities", Json::Array{Json::Object{}}},
    };
    CHECK(etude::writeScene(world, makeRegistry()) == expected);
}

TEST_CASE("readScene reads back what writeScene wrote, so that saving again gives the same text") {
    const etude::ComponentRegistry registry = makeRegistry();
    World world;
    const Entity player = world.create();
    const etude::Transform2D transform{
        .position = {0.1f, -2.5f},
        .rotation = 33.3f,
        .scale = {2.0f, 1.0f / 3.0f},
    };
    world.add(player, etude::Name{"Player"});
    world.add(player, transform);
    world.add(player, etude::SpriteRenderer{"sprites/bunny.qoi", 2});
    world.add(player, Health{42, 0.25f});
    world.add(world.create(), etude::Camera{2.0f});
    world.create();

    const std::string text = etude::writeJson(etude::writeScene(world, registry));
    const auto loaded = etude::readScene(etude::parseJson(text).value(), registry);
    REQUIRE(loaded);
    CHECK(loaded->size() == 3);
    CHECK(etude::writeJson(etude::writeScene(*loaded, registry)) == text);
}

TEST_CASE("A new component needs nothing but a type name and a field list to be saved and loaded") {
    const etude::ComponentRegistry registry = makeRegistry();
    World world;
    world.add(world.create(), Health{42, 0.25f});

    const std::string text = etude::writeJson(etude::writeScene(world, registry));
    const auto loaded = etude::readScene(etude::parseJson(text).value(), registry);
    REQUIRE(loaded);
    const Health& health = loaded->get<Health>(loaded->entities().front());
    CHECK(health.points == 42);
    CHECK(health.regeneration == 0.25f);
}

TEST_CASE("readScene refuses a scene without the right version") {
    CHECK(errorOf(R"({"entities": []})") == "the scene has to have version 1");
    CHECK(errorOf(R"({"version": 2, "entities": []})") == "the scene has to have version 1");
}

TEST_CASE("readScene refuses a scene without an array of entities") {
    CHECK(errorOf(R"({"version": 1})") == "the scene has to have an array of entities");
    CHECK(errorOf(R"({"version": 1, "entities": {}})") == "the scene has to have an array of entities");
}

TEST_CASE("readScene names the place in the scene that it cannot read") {
    CHECK(errorOf(R"({"version": 1, "entities": [{}, 3]})") == "entities[1] has to be an object");
    CHECK(errorOf(R"({"version": 1, "entities": [{"Speed": {}}]})") == "entities[0] has an unknown component Speed");
    CHECK(
        errorOf(R"({"version": 1, "entities": [{}, {"Camera": {"zoom": "far"}}]})") ==
        "entities[1].Camera.zoom: expected a number in the range of a float"
    );
}
