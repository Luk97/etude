#include <catch2/catch_test_macros.hpp>

#include <etude/ui/context.h>

#include <utility>

using etude::Input;
using etude::MouseButton;
using etude::PixelFont;
using etude::Vec2;
using etude::ui::Axis;
using etude::ui::Context;
using etude::ui::Id;
using etude::ui::makeId;
using etude::ui::rootId;
using etude::ui::Signal;
using etude::ui::SizeRule;

namespace {

    constexpr auto fontTexture = static_cast<etude::TextureId>(7);
    constexpr etude::Size screen{640, 360};
}

TEST_CASE("ui::makeId hashes the label with FNV-1a, starting from the parent") {
    STATIC_REQUIRE(makeId("", rootId) == rootId);
    STATIC_REQUIRE(makeId("a", rootId) == Id{0xaf63dc4c8601ec8c});
    STATIC_REQUIRE(makeId("foobar", rootId) == Id{0x85944171f73967e8});
    CHECK(makeId("OK", makeId("Panel", rootId)) != makeId("OK", rootId));
}

TEST_CASE("A part of the label after ## changes the ID, but does not show") {
    CHECK(etude::ui::visibleText("OK##2") == "OK");
    CHECK(etude::ui::visibleText("OK") == "OK");
    CHECK(makeId("OK##2", rootId) != makeId("OK", rootId));
}

TEST_CASE("Boxes line up from the top left corner, and the next frame tells where they were") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        const Signal first = ui.box({
            .label = "First",
            .width = SizeRule::pixels(50.0f),
            .height = SizeRule::pixels(20.0f),
        });
        const Signal second = ui.box({
            .label = "Second",
            .width = SizeRule::pixels(30.0f),
            .height = SizeRule::pixels(10.0f),
        });
        ui.endFrame();
        return std::pair{first, second};
    };
    CHECK(frame().first.rect.size == Vec2{});
    const auto [first, second] = frame();
    CHECK(first.rect.position == Vec2{0.0f, 0.0f});
    CHECK(first.rect.size == Vec2{50.0f, 20.0f});
    CHECK(second.rect.position == Vec2{0.0f, 20.0f});
    CHECK(second.rect.size == Vec2{30.0f, 10.0f});
}

TEST_CASE("A box that fits its children adds them up along its child axis and takes the largest across it") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        const Signal row = ui.beginBox({
            .label = "Row",
            .childAxis = Axis::X,
        });
        ui.box({
            .label = "Left",
            .width = SizeRule::pixels(50.0f),
            .height = SizeRule::pixels(20.0f),
        });
        const Signal right = ui.box({
            .label = "Right",
            .width = SizeRule::pixels(30.0f),
            .height = SizeRule::pixels(40.0f),
        });
        ui.endBox();
        ui.endFrame();
        return std::pair{row, right};
    };
    frame();
    const auto [row, right] = frame();
    CHECK(row.rect.size == Vec2{80.0f, 40.0f});
    CHECK(right.rect.position == Vec2{50.0f, 0.0f});
}

TEST_CASE("A share of the parent takes that part of the size of the parent") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Signal child;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 1);
        ui.beginBox({
            .label = "Parent",
            .width = SizeRule::pixels(200.0f),
            .height = SizeRule::pixels(100.0f),
        });
        child = ui.box({
            .label = "Child",
            .width = SizeRule::parentShare(0.5f),
            .height = SizeRule::parentShare(0.25f),
        });
        ui.endBox();
        ui.endFrame();
    }
    CHECK(child.rect.size == Vec2{100.0f, 25.0f});
}

TEST_CASE("A box that fits its text adds the padding, and the scale multiplies text, padding and pixels") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Signal text;
    Signal square;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 2);
        text = ui.box({
            .label = "Hasen",
            .width = SizeRule::fitText(4.0f),
            .height = SizeRule::fitText(2.0f),
        });
        square = ui.box({
            .label = "Square",
            .width = SizeRule::pixels(10.0f),
            .height = SizeRule::pixels(10.0f),
        });
        ui.endFrame();
    }
    CHECK(text.rect.size == Vec2{76.0f, 24.0f});
    CHECK(square.rect.size == Vec2{20.0f, 20.0f});
}

TEST_CASE("A button counts a click when the left button comes up over it") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        const bool clicked = ui.button("OK");
        ui.endFrame();
        input.beginFrame();
        return clicked;
    };
    input.onMouseMove({5.0f, 5.0f});
    CHECK_FALSE(frame());
    input.onMouseButtonDown(MouseButton::Left);
    CHECK_FALSE(frame());
    input.onMouseButtonUp(MouseButton::Left);
    CHECK(frame());
    CHECK_FALSE(frame());
}

TEST_CASE("Only the topmost clickable box under the mouse reacts, so a click on a button leaves the box below alone") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        const Signal panel = ui.beginBox({
            .label = "Panel",
            .width = SizeRule::pixels(100.0f),
            .height = SizeRule::pixels(100.0f),
            .clickable = true,
        });
        const bool button = ui.button("OK");
        ui.endBox();
        ui.endFrame();
        input.beginFrame();
        return std::pair{panel.clicked, button};
    };
    const auto clickAt = [&](Vec2 position) {
        input.onMouseMove(position);
        input.onMouseButtonDown(MouseButton::Left);
        frame();
        input.onMouseButtonUp(MouseButton::Left);
        return frame();
    };
    frame();
    CHECK(clickAt({5.0f, 5.0f}) == std::pair{false, true});
    CHECK(clickAt({50.0f, 50.0f}) == std::pair{true, false});
}

TEST_CASE("A box only counts a click that starts and ends over it, and stays held while the mouse is away") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Signal signal;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        signal = ui.box({
            .label = "OK",
            .width = SizeRule::pixels(20.0f),
            .height = SizeRule::pixels(20.0f),
            .clickable = true,
        });
        ui.endFrame();
        input.beginFrame();
    };
    frame();

    input.onMouseMove({30.0f, 5.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseMove({5.0f, 5.0f});
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    CHECK(signal.hovered);
    CHECK_FALSE(signal.clicked);

    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseMove({30.0f, 5.0f});
    frame();
    CHECK(signal.held);
    CHECK_FALSE(signal.hovered);
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    CHECK_FALSE(signal.clicked);
    CHECK_FALSE(signal.held);
}

TEST_CASE("While a box is held, no other box reacts to the mouse, not even when the button comes up over it") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Signal left;
    Signal right;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        ui.beginBox({
            .label = "Row",
            .childAxis = Axis::X,
        });
        left = ui.box({
            .label = "Left",
            .width = SizeRule::pixels(20.0f),
            .height = SizeRule::pixels(20.0f),
            .clickable = true,
        });
        right = ui.box({
            .label = "Right",
            .width = SizeRule::pixels(20.0f),
            .height = SizeRule::pixels(20.0f),
            .clickable = true,
        });
        ui.endBox();
        ui.endFrame();
        input.beginFrame();
    };
    frame();

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

    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    CHECK(right.clicked);
}

TEST_CASE("A button draws its background and then its text in the middle") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    ui.beginFrame(input, screen, 1);
    ui.button("OK");
    ui.endFrame();
    const auto sprites = ui.sprites();
    REQUIRE(sprites.size() == 3);
    CHECK(sprites[0].position == Vec2{0.0f, 0.0f});
    CHECK(sprites[0].size == Vec2{24.0f, 14.0f});
    CHECK(sprites[0].texture == etude::TextureId{});
    CHECK(sprites[1].position == Vec2{6.0f, 3.0f});
    CHECK(sprites[1].texture == fontTexture);
    CHECK(sprites[2].position == Vec2{12.0f, 3.0f});
}

TEST_CASE("A button turns lighter under the mouse and darker while it is held") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto background = [&] {
        ui.beginFrame(input, screen, 1);
        ui.button("OK");
        ui.endFrame();
        input.beginFrame();
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
