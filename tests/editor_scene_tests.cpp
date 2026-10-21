#include <catch2/catch_test_macros.hpp>

#include <etude/editor/scene/scene_pane.h>
#include <etude/math/rect.h>
#include <etude/math/vec2.h>
#include <etude/render2d/render_list.h>
#include <etude/rendering/sprite.h>
#include <etude/runtime/render_system.h>
#include <etude/scene/components/sprite_renderer.h>
#include <etude/scene/components/transform2d.h>
#include <etude/scene/world.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>

#include <optional>
#include <string>

using etude::Entity;
using etude::Input;
using etude::MouseButton;
using etude::TextureId;
using etude::Vec2;
using etude::World;

namespace {

    constexpr auto fontTexture = static_cast<TextureId>(7);
    constexpr auto bunnyTexture = static_cast<TextureId>(9);
    constexpr etude::Size screen{640, 360};

    std::optional<etude::LoadedTexture> fakeTexture(const std::string& path) {
        if (path != "bunny.qoi") {
            return std::nullopt;
        }
        return etude::LoadedTexture{
            .id = bunnyTexture,
            .size = {32.0f, 16.0f},
        };
    }

    // Shows a scene pane of 400 by 300 pixels with a bunny at the origin in frames of its own. The view inside it runs
    // from 4, 18 to 396, 296, so the origin of the world lies in its middle at 200, 157.
    struct SceneFrames {
        Input input;
        etude::ui::Context ui{etude::PixelFont{fontTexture}};
        World world;
        const etude::TextureLookup textures = fakeTexture;
        etude::editor::ScenePane scene{world, textures};
        std::optional<Entity> selection;
        Entity bunny = world.create();

        SceneFrames() {
            world.add(bunny, etude::Transform2D{});
            world.add(bunny, etude::SpriteRenderer{"bunny.qoi", 0});
            frame();
        }

        void frame() {
            ui.beginFrame(input, screen, 1);
            scene.show(ui, etude::ui::Modifier().size(400.0f, 300.0f), selection);
            ui.endFrame();
            input.beginFrame();
        }

        void click(MouseButton button, Vec2 position) {
            input.onMouseMove(position);
            input.onMouseButtonDown(button);
            frame();
            input.onMouseButtonUp(button);
            frame();
        }
    };
}

TEST_CASE("The scene pane selects the entity on top under a press of the left button, and nothing on empty space") {
    SceneFrames frames;
    frames.click(MouseButton::Left, {210.0f, 160.0f});
    CHECK(frames.selection == frames.bunny);
    frames.click(MouseButton::Left, {300.0f, 250.0f});
    CHECK(frames.selection == std::nullopt);
}

TEST_CASE("The scene pane draws through its own camera, which the middle button drags, between grid and frames") {
    SceneFrames frames;
    frames.selection = frames.bunny;

    // Dragging 20 pixels to the right moves the world along, from the frame after the press.
    frames.input.onMouseMove({100.0f, 100.0f});
    frames.input.onMouseButtonDown(MouseButton::Middle);
    frames.frame();
    frames.input.onMouseMove({120.0f, 100.0f});
    frames.frame();
    frames.input.onMouseButtonUp(MouseButton::Middle);
    frames.frame();

    // The list already holds a sprite of the world, which ends up above the grid. The camera shows 392 by 278 units
    // around its center, which now lies 20 units left of the origin.
    etude::RenderList list;
    list.sprites.push_back({
        .texture = bunnyTexture,
    });
    frames.scene.draw(list, frames.selection, screen);
    CHECK(list.camera.position == Vec2{-216.0f, -139.0f});
    CHECK(list.sprites.front().texture == TextureId{});
    CHECK(list.sprites.back().texture == bunnyTexture);

    // The frame of the selection lies in a batch of its own, cut off where the pane shows the world.
    REQUIRE(list.batches.size() == 1);
    CHECK(list.batches[0].sprites.size() == 4);
    const etude::Rect view{
        .position = {4.0f, 18.0f},
        .size = {392.0f, 278.0f},
    };
    CHECK(frames.scene.area() == view);
    CHECK(list.batches[0].clip == view);
}
