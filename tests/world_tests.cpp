#include <catch2/catch_test_macros.hpp>

#include <etude/scene/world.h>

using etude::Entity;
using etude::World;

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
