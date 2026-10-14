#include <catch2/catch_test_macros.hpp>

#include <etude/ui/box.h>
#include <etude/ui/context.h>
#include <etude/ui/label.h>
#include <etude/ui/modifier.h>
#include <etude/ui/row.h>

#include <utility>

using etude::Input;
using etude::MouseButton;
using etude::PixelFont;
using etude::Vec2;
using etude::ui::Context;
using etude::ui::Interaction;
using etude::ui::Modifier;

namespace {

    constexpr auto fontTexture = static_cast<etude::TextureId>(7);
    constexpr etude::Size screen{640, 360};

    constexpr etude::Color gray{
        .r = 0.5f,
        .g = 0.5f,
        .b = 0.5f,
    };
}

TEST_CASE("An element only counts a click that starts and ends over it, and stays held while the mouse is away") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction interaction;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        interaction = box(ui, Modifier().key("OK").size(20.0f, 20.0f).clickable());
        ui.endFrame();
        input.beginFrame();
    };
    frame();

    // A press beside the element and a release over it count as no click.
    input.onMouseMove({30.0f, 5.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseMove({5.0f, 5.0f});
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    CHECK(interaction.hovered);
    CHECK_FALSE(interaction.clicked);

    // A press over the element holds it while the mouse leaves, but a release beside it counts as no click either.
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseMove({30.0f, 5.0f});
    frame();
    CHECK(interaction.held);
    CHECK_FALSE(interaction.hovered);
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    CHECK_FALSE(interaction.clicked);
    CHECK_FALSE(interaction.held);

    // Only a press and a release over the element make a click.
    input.onMouseMove({5.0f, 5.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    CHECK(interaction.clicked);
}

TEST_CASE("Only the topmost clickable element under the mouse reacts, so a click leaves the element below alone") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        bool inner = false;
        const Interaction outer = box(ui, Modifier().key("Outer").size(100.0f, 100.0f).clickable(), [&] {
            inner = box(ui, Modifier().key("Inner").size(20.0f, 20.0f).clickable()).clicked;
        });
        ui.endFrame();
        input.beginFrame();
        return std::pair{outer.clicked, inner};
    };
    const auto clickAt = [&](Vec2 position) {
        input.onMouseMove(position);
        input.onMouseButtonDown(MouseButton::Left);
        frame();
        input.onMouseButtonUp(MouseButton::Left);
        return frame();
    };
    frame();

    // The inner element lies on top of the outer one, so within its 20 pixels it alone reacts.
    CHECK(clickAt({5.0f, 5.0f}) == std::pair{false, true});
    CHECK(clickAt({50.0f, 50.0f}) == std::pair{true, false});
}

TEST_CASE("While an element is held, no other element reacts to the mouse, not even when the button comes up over it") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction left;
    Interaction right;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        row(ui, Modifier().key("Row"), [&] {
            left = box(ui, Modifier().key("Left").size(20.0f, 20.0f).clickable());
            right = box(ui, Modifier().key("Right").size(20.0f, 20.0f).clickable());
        });
        ui.endFrame();
        input.beginFrame();
    };
    frame();

    // The press on the left element holds it, so the right one neither lights up nor counts the release over it.
    input.onMouseMove({5.0f, 5.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseMove({25.0f, 5.0f});
    frame();
    CHECK_FALSE(right.hovered);
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    CHECK_FALSE(left.clicked);
    CHECK_FALSE(right.clicked);

    // Once the left element is let go, a new press over the right one clicks it.
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    CHECK(right.clicked);
}

TEST_CASE("A clickable element turns lighter under the mouse and darker while it is held") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto background = [&] {
        ui.beginFrame(input, screen, 1);
        box(ui, Modifier().key("OK").size(20.0f, 20.0f).background(gray).clickable());
        ui.endFrame();
        input.beginFrame();

        // The background is the first sprite, and in a gray the red channel tells how light it is.
        return ui.sprites()[0].color.r;
    };
    const float normal = background();
    input.onMouseMove({5.0f, 5.0f});
    const float hovered = background();
    input.onMouseButtonDown(MouseButton::Left);
    const float held = background();
    CHECK(hovered > normal);
    CHECK(held < normal);
}

TEST_CASE("A layout hands its interaction to its content while the content adds the children") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    bool hoveredInside = false;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        const Interaction outer =
            box(ui, Modifier().key("Outer").size(50.0f, 50.0f).clickable(),
                [&](const Interaction& self) { hoveredInside = self.hovered; });
        ui.endFrame();
        input.beginFrame();
        return outer.hovered;
    };
    frame();
    input.onMouseMove({10.0f, 10.0f});
    CHECK(frame());
    CHECK(hoveredInside);
    input.onMouseMove({100.0f, 100.0f});
    CHECK_FALSE(frame());
    CHECK_FALSE(hoveredInside);
}

TEST_CASE("Equal keys in different layouts name different elements") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        bool upper = false;
        bool lower = false;
        row(ui, Modifier().key("Upper"),
            [&] { upper = box(ui, Modifier().key("OK").size(20.0f, 20.0f).clickable()).clicked; });
        row(ui, Modifier().key("Lower"),
            [&] { lower = box(ui, Modifier().key("OK").size(20.0f, 20.0f).clickable()).clicked; });
        ui.endFrame();
        input.beginFrame();
        return std::pair{upper, lower};
    };
    frame();

    // The mouse at y 25 lies over the OK of the lower row.
    input.onMouseMove({5.0f, 25.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseButtonUp(MouseButton::Left);
    CHECK(frame() == std::pair{false, true});
}

TEST_CASE("Keys that only join to the same text still name different elements") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        bool first = false;
        bool second = false;
        row(ui, Modifier().key("1"),
            [&] { first = box(ui, Modifier().key("10").size(20.0f, 20.0f).clickable()).clicked; });
        row(ui, Modifier().key("11"),
            [&] { second = box(ui, Modifier().key("0").size(20.0f, 20.0f).clickable()).clicked; });
        ui.endFrame();
        input.beginFrame();
        return std::pair{first, second};
    };
    frame();

    // Both pairs of keys join to 110, but the mouse at y 25 only clicks the element of the second row.
    input.onMouseMove({5.0f, 25.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseButtonUp(MouseButton::Left);
    CHECK(frame() == std::pair{false, true});
}

TEST_CASE("Only the part of a label before ## shows") {
    STATIC_REQUIRE(etude::ui::visibleText("OK##2") == "OK");
    STATIC_REQUIRE(etude::ui::visibleText("OK") == "OK");
    STATIC_REQUIRE(etude::ui::visibleText("##hidden").empty());
}

TEST_CASE("The UI wants the mouse over what it drew, and a press belongs to where it began until it ends") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    bool drawn = true;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        if (drawn) {
            box(ui, Modifier().size(50.0f, 20.0f).background(gray));
        }

        // The second box draws nothing.
        box(ui, Modifier().size(50.0f, 20.0f));
        ui.endFrame();
        const bool wanted = ui.wantsMouse();
        input.beginFrame();
        return wanted;
    };

    // The UI only knows what it drew once its first frame has ended.
    input.onMouseMove({10.0f, 10.0f});
    CHECK_FALSE(frame());
    CHECK(frame());
    input.onMouseMove({10.0f, 30.0f});
    CHECK_FALSE(frame());

    // A press over the UI keeps the mouse with the UI wherever it moves, up to the frame in which it ends.
    input.onMouseMove({10.0f, 10.0f});
    input.onMouseButtonDown(MouseButton::Left);
    CHECK(frame());
    input.onMouseMove({300.0f, 300.0f});
    CHECK(frame());
    input.onMouseButtonUp(MouseButton::Left);
    CHECK(frame());
    CHECK_FALSE(frame());

    // A press of the game keeps the mouse with the game, even over the UI.
    input.onMouseButtonDown(MouseButton::Left);
    CHECK_FALSE(frame());
    input.onMouseMove({10.0f, 10.0f});
    CHECK_FALSE(frame());
    input.onMouseButtonUp(MouseButton::Left);
    CHECK_FALSE(frame());
    CHECK(frame());

    // An element that the UI no longer draws lets go of the mouse after one frame.
    drawn = false;
    CHECK(frame());
    CHECK_FALSE(frame());
}
