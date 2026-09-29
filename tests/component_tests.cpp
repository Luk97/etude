#include <catch2/catch_test_macros.hpp>

#include <etude/json/json.h>
#include <etude/scene/component_json.h>
#include <etude/scene/component_registry.h>
#include <etude/scene/components.h>
#include <etude/scene/reflection.h>
#include <etude/scene/world.h>

#include <array>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

using etude::Json;

namespace {

    Json json(std::string_view text) {
        return etude::parseJson(text).value();
    }

    template <typename Component>
    std::string errorOf(std::string_view text) {
        const auto read = etude::readComponent<Component>(json(text));
        REQUIRE_FALSE(read);
        return read.error();
    }

    // A component whose fields have no default values, as one written in a hurry might look.
    struct Bare {
        float value;
        int count;

        static constexpr std::string_view typeName = "Bare";
        static constexpr std::tuple fields{
            etude::Field{"value", &Bare::value},
            etude::Field{"count", &Bare::count},
        };
    };
}

TEST_CASE("writeComponent writes the fields in the order of the field list") {
    const etude::Transform2D transform{
        .position = {0.1f, -2.0f},
        .rotation = 90.0f,
    };
    CHECK(etude::writeJson(etude::writeComponent(transform)) == R"({
    "position": [0.1, -2],
    "rotation": 90,
    "scale": [1, 1]
})");
}

TEST_CASE("readComponent reads back what writeComponent wrote") {
    const etude::Transform2D transform{
        .position = {0.1f, -2.0f},
        .rotation = 33.3f,
        .scale = {1.0f / 3.0f, 2.0f},
    };
    const Json written = json(etude::writeJson(etude::writeComponent(transform)));
    const auto read = etude::readComponent<etude::Transform2D>(written);
    REQUIRE(read);
    CHECK(read->position == transform.position);
    CHECK(read->rotation == transform.rotation);
    CHECK(read->scale == transform.scale);
}

TEST_CASE("readComponent keeps the default values of the fields that the object leaves out") {
    const auto read = etude::readComponent<etude::Transform2D>(json(R"({"rotation": 45})"));
    REQUIRE(read);
    CHECK(read->rotation == 45.0f);
    CHECK(read->scale == etude::Vec2{1.0f, 1.0f});
}

TEST_CASE("readComponent starts from zero for fields without a default value") {
    const auto read = etude::readComponent<Bare>(json("{}"));
    REQUIRE(read);
    CHECK(read->value == 0.0f);
    CHECK(read->count == 0);
}

TEST_CASE("readComponent refuses a member that is no field") {
    CHECK(errorOf<etude::Transform2D>(R"({"positon": [1, 2]})") == "Transform2D has no field positon");
}

TEST_CASE("readComponent names the field whose value has the wrong type") {
    CHECK(errorOf<etude::Transform2D>("[]") == "Transform2D has to be an object");
    CHECK(
        errorOf<etude::Transform2D>(R"({"rotation": "90"})") ==
        "Transform2D.rotation: expected a number in the range of a float"
    );
    const std::string wrongScale = "Transform2D.scale: expected an array of two numbers";
    CHECK(errorOf<etude::Transform2D>(R"({"scale": [1]})") == wrongScale);
    CHECK(errorOf<etude::Transform2D>(R"({"scale": [1, 2, 3]})") == wrongScale);
    CHECK(errorOf<etude::Name>(R"({"value": 7})") == "Name.value: expected a string");
}

TEST_CASE("readComponent refuses numbers that do not fit the type of the field") {
    const auto read = etude::readComponent<etude::SpriteRenderer>(json(R"({"texture": "bunny.qoi", "layer": -3})"));
    REQUIRE(read);
    CHECK(read->texture == "bunny.qoi");
    CHECK(read->layer == -3);

    const std::string wrongLayer = "SpriteRenderer.layer: expected a whole number in the range of an int";
    CHECK(errorOf<etude::SpriteRenderer>(R"({"layer": 1.5})") == wrongLayer);
    CHECK(errorOf<etude::SpriteRenderer>(R"({"layer": 3e9})") == wrongLayer);
    CHECK(errorOf<etude::Camera>(R"({"zoom": 1e39})") == "Camera.zoom: expected a number in the range of a float");
}

TEST_CASE("Floats keep their value and their short form on the way through JSON text") {
    CHECK(etude::writeJson(etude::toJson(0.1f)) == "0.1");

    // The shortest digits of 7.038531e-26f turn into the neighboring float when read as a double.
    const std::array values{
        0.1f,
        1.0f / 3.0f,
        16777216.0f,
        7.038531e-26f,
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::min(),
        std::numeric_limits<float>::denorm_min(),
    };
    for (const float value : values) {
        float back = 0.0f;
        REQUIRE(etude::fromJson(json(etude::writeJson(etude::toJson(value))), back));
        CHECK(back == value);
    }
}

TEST_CASE("hasField knows the fields of a component at compile time") {
    STATIC_REQUIRE(etude::hasField<etude::Transform2D>("scale"));
    STATIC_REQUIRE_FALSE(etude::hasField<etude::Transform2D>("size"));
}

TEST_CASE("addBuiltinComponents adds the components of the engine in a fixed order") {
    etude::ComponentRegistry registry;
    etude::addBuiltinComponents(registry);
    std::vector<std::string_view> names;
    for (const etude::ComponentType& type : registry.types()) {
        names.push_back(type.name);
    }
    CHECK(names == std::vector<std::string_view>{"Name", "Transform2D", "SpriteRenderer", "Camera"});
    CHECK(registry.find("Camera") == &registry.types()[3]);
    CHECK(registry.find("Camera2D") == nullptr);
}

TEST_CASE("ComponentRegistry writes and reads the component of an entity by the name of its type") {
    etude::ComponentRegistry registry;
    etude::addBuiltinComponents(registry);
    const etude::ComponentType& nameType = *registry.find("Name");

    etude::World world;
    const etude::Entity player = world.create();
    world.add(player, etude::Name{"Player"});
    const std::optional<Json> written = nameType.write(world, player);
    REQUIRE(written);
    CHECK(*written == Json(Json::Object{{"value", "Player"}}));
    CHECK_FALSE(registry.find("Camera")->write(world, player));

    const etude::Entity copy = world.create();
    REQUIRE(nameType.read(world, copy, *written));
    CHECK(world.get<etude::Name>(copy).value == "Player");
}

TEST_CASE("ComponentRegistry adds nothing to the entity when reading fails") {
    etude::ComponentRegistry registry;
    etude::addBuiltinComponents(registry);
    etude::World world;
    const etude::Entity entity = world.create();
    CHECK_FALSE(registry.find("Camera")->read(world, entity, json(R"({"zoom": "near"})")));
    CHECK_FALSE(world.has<etude::Camera>(entity));
}
