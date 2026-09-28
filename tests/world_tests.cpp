#include <catch2/catch_test_macros.hpp>

#include <etude/scene/world.h>

#include <memory>
#include <string>
#include <type_traits>

using etude::Entity;
using etude::World;

namespace {

    struct Position {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct Label {
        std::string text;
    };
}

TEST_CASE("Entity packs index and generation into 32 bits") {
    STATIC_REQUIRE(sizeof(Entity) == 4);
    constexpr Entity entity = etude::makeEntity(5, 3);
    STATIC_REQUIRE(etude::indexOf(entity) == 5);
    STATIC_REQUIRE(etude::generationOf(entity) == 3);
}

TEST_CASE("World creates distinct entities that are alive") {
    World world;
    const Entity first = world.create();
    const Entity second = world.create();
    CHECK(first != second);
    CHECK(world.alive(first));
    CHECK(world.alive(second));
    CHECK(world.size() == 2);
}

TEST_CASE("World gives the index of a destroyed entity to a new one with the next generation") {
    World world;
    const Entity old = world.create();
    world.destroy(old);
    const Entity reused = world.create();
    CHECK(etude::indexOf(reused) == etude::indexOf(old));
    CHECK(etude::generationOf(reused) == etude::generationOf(old) + 1);
    CHECK_FALSE(world.alive(old));
    CHECK(world.alive(reused));
}

TEST_CASE("World ignores destroying an entity twice") {
    World world;
    const Entity entity = world.create();
    world.destroy(entity);
    world.destroy(entity);
    CHECK(world.size() == 0);

    // Had the second destroy freed the index again, both new entities would get it.
    const Entity first = world.create();
    const Entity second = world.create();
    CHECK(first != second);
}

TEST_CASE("World does not know the entities of another world") {
    World one;
    const World other;
    CHECK_FALSE(other.alive(one.create()));
}

TEST_CASE("World starts the generation of an index again at 0 after 4096 reuses") {
    World world;
    const Entity first = world.create();
    Entity entity = first;
    for (int i = 0; i < 4096; ++i) {
        world.destroy(entity);
        entity = world.create();
    }

    // The known limit of 12 bits of generation: the very first handle looks alive again.
    CHECK(entity == first);
    CHECK(world.alive(first));
}

TEST_CASE("World adds components and finds them again") {
    World world;
    const Entity entity = world.create();
    world.add(entity, Position{1.0f, 2.0f});
    world.add(entity, Label{"player"});
    CHECK(world.get<Position>(entity).y == 2.0f);
    CHECK(world.get<Label>(entity).text == "player");
}

TEST_CASE("World knows which entities have a component") {
    World world;
    const Entity unlabeled = world.create();
    const Entity labeled = world.create();
    world.add(labeled, Label{"labeled"});
    CHECK(world.has<Label>(labeled));
    CHECK_FALSE(world.has<Label>(unlabeled));
    CHECK_FALSE(world.has<Position>(labeled));
    CHECK(world.tryGet<Position>(labeled) == nullptr);
}

TEST_CASE("World replaces a component that the entity already has") {
    World world;
    const Entity entity = world.create();
    world.add(entity, Label{"first"});
    world.add(entity, Label{"second"});
    CHECK(world.get<Label>(entity).text == "second");
}

TEST_CASE("World removes a component and keeps those of the other entities") {
    World world;
    const Entity first = world.create();
    const Entity second = world.create();
    const Entity third = world.create();
    world.add(first, Position{1.0f, 0.0f});
    world.add(second, Position{2.0f, 0.0f});
    world.add(third, Position{3.0f, 0.0f});

    // The last component moves into the gap, so the storage has to note its new place.
    world.remove<Position>(first);
    CHECK_FALSE(world.has<Position>(first));
    CHECK(world.get<Position>(second).x == 2.0f);
    CHECK(world.get<Position>(third).x == 3.0f);
}

TEST_CASE("World ignores removing a component that the entity does not have") {
    World world;
    const Entity entity = world.create();
    world.remove<Position>(entity);
    world.add(entity, Position{});
    world.remove<Position>(entity);
    world.remove<Position>(entity);
    CHECK_FALSE(world.has<Position>(entity));
}

TEST_CASE("World releases the components of an entity that it destroys") {
    World world;
    const Entity entity = world.create();
    auto resource = std::make_shared<int>(0);
    const std::weak_ptr<int> watcher = resource;
    world.add(entity, std::move(resource));
    world.destroy(entity);
    CHECK(watcher.expired());
}

TEST_CASE("World does not show the component of a new entity to an old handle with the same index") {
    World world;
    const Entity old = world.create();
    world.destroy(old);
    const Entity reused = world.create();
    world.add(reused, Position{});
    CHECK_FALSE(world.has<Position>(old));
}

TEST_CASE("World hands out const components from a const world") {
    World world;
    const Entity entity = world.create();
    world.add(entity, Position{});
    const World& constant = world;
    STATIC_REQUIRE(std::is_same_v<decltype(world.get<Position>(entity)), Position&>);
    STATIC_REQUIRE(std::is_same_v<decltype(constant.get<Position>(entity)), const Position&>);
    CHECK(&constant.get<Position>(entity) == &world.get<Position>(entity));
}
