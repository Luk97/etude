#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <etude/render2d/render_list.h>
#include <etude/runtime/render_system.h>
#include <etude/scene/components/camera.h>
#include <etude/scene/components/sprite_renderer.h>
#include <etude/scene/components/transform2d.h>
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

    const Vec2 viewport{800.0f, 600.0f};

    std::optional<LoadedTexture> fakeTexture(const std::string& path) {
        if (path != "bunny.qoi") {
            return std::nullopt;
        }
        return LoadedTexture{
            .id = TextureId{7},
            .size = {32.0f, 16.0f},
        };
    }

    Entity addBunny(World& world, float x, int layer) {
        const Entity entity = world.create();
        const etude::Transform2D transform{
            .position = {x, 0.0f},
        };
        world.add(entity, transform);
        world.add(entity, etude::SpriteRenderer{"bunny.qoi", layer});
        return entity;
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
    etude::drawScene(world, viewport, fakeTexture, list);
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
    etude::drawScene(world, viewport, fakeTexture, list);
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
    etude::drawScene(world, viewport, fakeTexture, list);
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
    etude::drawScene(world, viewport, fakeTexture, list);
    CHECK(list.camera.position == Vec2{200.0f, 150.0f});
    CHECK(list.camera.zoom == 2.0f);
}

TEST_CASE("drawScene takes the Camera with the lowest index and keeps the camera of the list without one") {
    World world;
    RenderList list;
    list.camera.zoom = 3.0f;
    etude::drawScene(world, viewport, fakeTexture, list);
    CHECK(list.camera.zoom == 3.0f);

    const Entity first = world.create();
    const Entity second = world.create();
    world.add(second, etude::Transform2D{});
    world.add(second, etude::Camera{4.0f});
    world.add(first, etude::Transform2D{});
    world.add(first, etude::Camera{2.0f});
    etude::drawScene(world, viewport, fakeTexture, list);
    CHECK(list.camera.zoom == 2.0f);
}

TEST_CASE("drawScene skips a Camera without a positive zoom") {
    World world;
    const Entity broken = world.create();
    world.add(broken, etude::Transform2D{});
    world.add(broken, etude::Camera{0.0f});
    const Entity working = world.create();
    world.add(working, etude::Transform2D{});
    world.add(working, etude::Camera{2.0f});

    RenderList list;
    etude::drawScene(world, viewport, fakeTexture, list);
    CHECK(list.camera.zoom == 2.0f);
}

TEST_CASE("entityAt finds the entity on top under the point, by layer first and then by index") {
    World world;
    const Entity back = addBunny(world, 0.0f, 0);
    const Entity front = addBunny(world, 40.0f, 1);
    const Entity later = addBunny(world, 20.0f, 0);

    // Each bunny covers 32 by 16 units around its position: the back one from x -16 to 16, the later one from 4 to 36
    // and the front one from 24 to 56. The later one covers the back one by its index, the front one covers the later
    // one by its layer, although its index is lower.
    CHECK(etude::entityAt(world, {-10.0f, 0.0f}, fakeTexture) == back);
    CHECK(etude::entityAt(world, {10.0f, 0.0f}, fakeTexture) == later);
    CHECK(etude::entityAt(world, {30.0f, 4.0f}, fakeTexture) == front);
    CHECK(etude::entityAt(world, {0.0f, 20.0f}, fakeTexture) == std::nullopt);
}

TEST_CASE("entityAt turns the point with the sprite, so that only the turned rectangle counts") {
    World world;
    const Entity entity = world.create();
    const etude::Transform2D transform{
        .rotation = 30.0f,
    };
    world.add(entity, transform);
    world.add(entity, etude::SpriteRenderer{"bunny.qoi", 0});

    // Turned clockwise, the long side of 32 units points to the right and down, since y points down. A point 14 units
    // along it lies inside, its mirror image above the x axis lies outside, though both are inside the unturned sprite.
    CHECK(etude::entityAt(world, {12.12f, 7.0f}, fakeTexture) == entity);
    CHECK(etude::entityAt(world, {12.12f, -7.0f}, fakeTexture) == std::nullopt);
}

TEST_CASE("entityAt finds mirrored sprites and skips entities without a texture") {
    World world;
    const Entity mirrored = world.create();
    const etude::Transform2D transform{
        .scale = {-2.0f, 1.0f},
    };
    world.add(mirrored, transform);
    world.add(mirrored, etude::SpriteRenderer{"bunny.qoi", 0});
    const Entity invisible = world.create();
    world.add(invisible, etude::Transform2D{});
    world.add(invisible, etude::SpriteRenderer{"missing.qoi", 5});

    CHECK(etude::entityAt(world, {30.0f, 0.0f}, fakeTexture) == mirrored);
    CHECK(etude::entityAt(world, {0.0f, 0.0f}, fakeTexture) == mirrored);
}
