#include <catch2/catch_test_macros.hpp>

#include <etude/editor/hierarchy_pane.h>
#include <etude/editor/widgets/pane.h>
#include <etude/scene/components/name.h>
#include <etude/scene/world.h>
#include <etude/ui/box.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>

#include <optional>

using etude::Entity;
using etude::Input;
using etude::MouseButton;
using etude::PixelFont;
using etude::Vec2;
using etude::World;
using etude::ui::Context;
using etude::ui::Interaction;
using etude::ui::Modifier;

namespace {

    constexpr auto fontTexture = static_cast<etude::TextureId>(7);
    constexpr etude::Size screen{640, 360};
}

TEST_CASE("A pane shows its title as a bar above the content, which fills the rest inside its padding") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction body;
    Interaction inside;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 1);
        body = etude::editor::pane(ui, "Test", Modifier().size(200.0f, 100.0f), [&] {
            inside = box(ui, Modifier().key("Inside").fillMaxSize());
        });
        ui.endFrame();
    }

    // The bar takes a line of 14 pixels, and the padding of 4 pixels runs around the content.
    CHECK(body.rect.position == Vec2{0.0f, 14.0f});
    CHECK(body.rect.size == Vec2{200.0f, 86.0f});
    CHECK(inside.rect.position == Vec2{4.0f, 18.0f});
    CHECK(inside.rect.size == Vec2{192.0f, 78.0f});
}

TEST_CASE("The hierarchy lists every entity and selects the one that is clicked") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    World world;
    world.add(world.create(), etude::Name{"Ball"});
    const Entity unnamed = world.create();
    std::optional<Entity> selection;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        etude::editor::hierarchyPane(ui, Modifier().size(200.0f, 100.0f), world, selection);
        ui.endFrame();
        input.beginFrame();
    };
    frame();

    // The lines are 14 pixels high and 3 apart, below the bar and the padding: the second one runs from 35 to 49.
    input.onMouseMove({20.0f, 40.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    CHECK(selection == unnamed);
}
