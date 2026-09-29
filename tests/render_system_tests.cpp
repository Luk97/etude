#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <etude/render2d/render_list.h>
#include <etude/runtime/render_system.h>
#include <etude/scene/components.h>
#include <etude/scene/world.h>

#include <numbers>
#include <optional>
#include <string>
#include <vector>

using etude::Entity;
using etude::LoadedTexture;
using etude::RenderList;
using etude::TextureId;
using etude::Vec2;
using etude::World;

namespace {

    const etude::Size window{800, 600};

    std::optional<LoadedTexture> fakeTexture(const std::string& path) {
        if (path != "bunny.qoi") {
            return std::nullopt;
        }
        return LoadedTexture{
            .id = TextureId{7},
            .size = {32.0f, 16.0f},
        };
    }

    void addBunny(World& world, float x, int layer) {
        const Entity entity = world.create();
        const etude::Transform2D transform{
            .position = {x, 0.0f},
        };
        world.add(entity, transform);
        world.add(entity, etude::SpriteRenderer{"bunny.qoi", layer});
    }
}

TEST_CASE("drawScene centers the sprite on its entity, stretches it by the scale and turns it in radians") {
    World world;
    const Entity entity = world.create();
    const etude::Transform2D transform{
        .position = {100.0f, 50.0f},
        .rotation = 90.0f,
        .scale = {2.0f, 1.0f},
    };
    world.add(entity, transform);
    world.add(entity, etude::SpriteRenderer{"bunny.qoi", 0});

    RenderList list;
    etude::drawScene(world, window, fakeTexture, list);
    REQUIRE(list.sprites.size() == 1);
    const etude::Sprite& sprite = list.sprites.front();
    CHECK(sprite.size == Vec2{64.0f, 16.0f});
    CHECK(sprite.position == Vec2{68.0f, 42.0f});
    CHECK_THAT(sprite.rotation, Catch::Matchers::WithinAbs(std::numbers::pi_v<float> / 2.0f, 1e-6));
    CHECK(sprite.texture == TextureId{7});
}

TEST_CASE("drawScene orders the sprites by layer and within a layer by entity index") {
    World world;
    addBunny(world, 0.0f, 1);
    addBunny(world, 10.0f, 0);
    addBunny(world, 20.0f, 1);

    RenderList list;
    etude::drawScene(world, window, fakeTexture, list);
    std::vector<float> centers;
    for (const etude::Sprite& sprite : list.sprites) {
        centers.push_back(sprite.position.x + sprite.size.x / 2.0f);
    }
    CHECK(centers == std::vector{10.0f, 0.0f, 20.0f});
}

TEST_CASE("drawScene leaves out the entities whose texture cannot be loaded") {
    World world;
    const Entity entity = world.create();
    world.add(entity, etude::Transform2D{});
    world.add(entity, etude::SpriteRenderer{"missing.qoi", 0});

    RenderList list;
    etude::drawScene(world, window, fakeTexture, list);
    CHECK(list.sprites.empty());
}

TEST_CASE("drawScene points the camera of the list like the Camera of the scene, which looks at the middle") {
    World world;
    const Entity camera = world.create();
    const etude::Transform2D transform{
        .position = {400.0f, 300.0f},
    };
    world.add(camera, transform);
    world.add(camera, etude::Camera{2.0f});

    RenderList list;
    etude::drawScene(world, window, fakeTexture, list);
    CHECK(list.camera.position == Vec2{200.0f, 150.0f});
    CHECK(list.camera.zoom == 2.0f);
}

TEST_CASE("drawScene takes the Camera with the lowest index and keeps the camera of the list without one") {
    World world;
    RenderList list;
    list.camera.zoom = 3.0f;
    etude::drawScene(world, window, fakeTexture, list);
    CHECK(list.camera.zoom == 3.0f);

    const Entity first = world.create();
    const Entity second = world.create();
    world.add(second, etude::Transform2D{});
    world.add(second, etude::Camera{4.0f});
    world.add(first, etude::Transform2D{});
    world.add(first, etude::Camera{2.0f});
    etude::drawScene(world, window, fakeTexture, list);
    CHECK(list.camera.zoom == 2.0f);
}
