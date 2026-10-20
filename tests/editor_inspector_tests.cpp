#include <catch2/catch_test_macros.hpp>

#include <etude/editor/inspector_pane.h>
#include <etude/scene/component_registry.h>
#include <etude/scene/components/name.h>
#include <etude/scene/components/transform2d.h>
#include <etude/scene/world.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>

using etude::Entity;
using etude::Input;
using etude::MouseButton;
using etude::PixelFont;
using etude::World;
using etude::ui::Context;
using etude::ui::Modifier;

namespace {

    constexpr auto fontTexture = static_cast<etude::TextureId>(7);
    constexpr etude::Size screen{640, 360};

    etude::ComponentRegistry makeRegistry() {
        etude::ComponentRegistry registry;
        etude::addBuiltinComponents(registry);
        return registry;
    }

    // Builds the inspector in frames of its own and lets a test press, drag and release the mouse between them.
    struct InspectorFrames {
        Input input;
        Context ui{PixelFont{fontTexture}};
        const etude::ComponentRegistry registry = makeRegistry();
        World world;
        Entity entity = world.create();

        void frame() {
            ui.beginFrame(input, screen, 1);
            etude::editor::inspectorPane(ui, Modifier().size(200.0f, 300.0f), world, registry, entity);
            ui.endFrame();
            input.beginFrame();
        }

        void click(etude::Vec2 position) {
            input.onMouseMove(position);
            input.onMouseButtonDown(MouseButton::Left);
            frame();
            input.onMouseButtonUp(MouseButton::Left);
            frame();
        }
    };
}

TEST_CASE("The inspector shows the fields of a component, and dragging a name changes its field right away") {
    InspectorFrames inspector;
    inspector.world.add(inspector.entity, etude::Transform2D{});
    inspector.frame();

    // Below the title and the padding lie the bar of Transform2D from y 18 to 32, the position from 35 to 49 and the
    // rotation from 52 to 66, each 3 pixels apart.
    inspector.input.onMouseMove({10.0f, 58.0f});
    inspector.input.onMouseButtonDown(MouseButton::Left);
    inspector.frame();
    inspector.input.onMouseMove({20.0f, 58.0f});
    inspector.frame();
    CHECK(inspector.world.get<etude::Transform2D>(inspector.entity).rotation == 10.0f);
}

TEST_CASE("The inspector adds a missing component with a button and removes one with the button of its bar") {
    InspectorFrames inspector;
    inspector.world.add(inspector.entity, etude::Name{"Ball"});
    inspector.frame();

    // The column of Name holds its bar and the field value, so the first button for a missing type follows from 52.
    inspector.click({10.0f, 58.0f});
    CHECK(inspector.world.has<etude::Transform2D>(inspector.entity));

    // The button x starts the bar of Name, from x 4 to 22.
    inspector.click({10.0f, 25.0f});
    CHECK_FALSE(inspector.world.has<etude::Name>(inspector.entity));
    CHECK(inspector.world.has<etude::Transform2D>(inspector.entity));
}
