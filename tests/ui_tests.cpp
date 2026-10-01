#include <catch2/catch_test_macros.hpp>

#include <etude/ui/context.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <string>
#include <utility>

using etude::Input;
using etude::Key;
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

    // Runs frames of a UI with one text field, like the game loop does. Its label Name takes 30 pixels, so the field
    // starts at x 30.
    struct TextFieldFrames {
        Input input;
        Context ui{PixelFont{fontTexture}};
        std::string text;

        bool frame() {
            ui.beginFrame(input, screen, 1);
            const bool changed = ui.textField("Name", text);
            ui.endFrame();
            input.beginFrame();
            return changed;
        }

        void clickAt(Vec2 position) {
            input.onMouseMove(position);
            input.onMouseButtonDown(MouseButton::Left);
            frame();
            input.onMouseButtonUp(MouseButton::Left);
            frame();
        }

        void tap(Key key) {
            input.onKeyDown(key);
            frame();
            input.onKeyUp(key);
        }

        bool type(char16_t character) {
            input.onCharacter(character);
            return frame();
        }
    };
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

TEST_CASE("Labels and text fields show ## in their text as it is") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    std::string text = "x##y";
    ui.beginFrame(input, screen, 1);
    ui.label("a##b");
    ui.textField("Name", text);
    ui.endFrame();
    const auto glyphs =
        std::ranges::count_if(ui.sprites(), [](const etude::Sprite& sprite) { return sprite.texture == fontTexture; });
    CHECK(glyphs == 12);
}

TEST_CASE("A checkbox flips its value when a click on the square or on its text ends") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    bool value = false;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        const bool changed = ui.checkbox("Pause", value);
        ui.endFrame();
        input.beginFrame();
        return changed;
    };
    const auto clickAt = [&](Vec2 position) {
        input.onMouseMove(position);
        input.onMouseButtonDown(MouseButton::Left);
        frame();
        input.onMouseButtonUp(MouseButton::Left);
        return frame();
    };
    frame();

    input.onMouseMove({5.0f, 5.0f});
    input.onMouseButtonDown(MouseButton::Left);
    CHECK_FALSE(frame());
    CHECK_FALSE(value);
    input.onMouseButtonUp(MouseButton::Left);
    CHECK(frame());
    CHECK(value);

    CHECK(clickAt({30.0f, 5.0f}));
    CHECK_FALSE(value);
}

TEST_CASE("A slider takes the value where the mouse presses or drags it, within its range") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    float value = 0.0f;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        const bool changed = ui.slider("Tempo", value, 0.0f, 2.0f);
        ui.endFrame();
        input.beginFrame();
        return changed;
    };
    frame();

    // The text Tempo takes 36 pixels, so the bar runs from x 36 to 136.
    input.onMouseMove({86.0f, 5.0f});
    input.onMouseButtonDown(MouseButton::Left);
    CHECK(frame());
    CHECK(value == 1.0f);
    input.onMouseMove({500.0f, 5.0f});
    frame();
    CHECK(value == 2.0f);
    input.onMouseButtonUp(MouseButton::Left);
    CHECK_FALSE(frame());
    CHECK(value == 2.0f);
}

TEST_CASE("A text field takes typed text from a click on it until Enter, Escape or a click elsewhere") {
    TextFieldFrames field{
        .text = "Hase",
    };
    field.frame();
    CHECK_FALSE(field.type(u'x'));

    field.clickAt({60.0f, 5.0f});
    CHECK(field.type(u'n'));
    field.tap(Key::Enter);
    field.type(u'x');
    CHECK(field.text == "Hasen");

    field.clickAt({60.0f, 5.0f});
    field.tap(Key::Escape);
    field.type(u'x');
    CHECK(field.text == "Hasen");

    field.clickAt({60.0f, 5.0f});
    field.clickAt({400.0f, 300.0f});
    field.type(u'x');
    CHECK(field.text == "Hasen");
}

TEST_CASE("Backspace, Delete, the arrow keys, Home and End edit the text character by character") {
    TextFieldFrames field{
        .text = "aä€",
    };
    field.frame();
    field.clickAt({60.0f, 5.0f});
    field.tap(Key::Backspace);
    CHECK(field.text == "aä");
    field.tap(Key::Left);
    field.tap(Key::Delete);
    CHECK(field.text == "a");
    field.tap(Key::Home);
    field.type(u'b');
    field.tap(Key::Right);
    field.type(u'c');
    CHECK(field.text == "bac");
    field.tap(Key::Home);
    field.tap(Key::End);
    field.type(u'ü');
    CHECK(field.text == "bacü");
}

TEST_CASE("A held Backspace removes one more character whenever the keyboard repeats it") {
    TextFieldFrames field{
        .text = "Hasen",
    };
    field.frame();
    field.clickAt({60.0f, 5.0f});
    field.input.onKeyDown(Key::Backspace);
    field.frame();
    CHECK(field.text == "Hase");
    field.frame();
    CHECK(field.text == "Hase");
    field.input.onKeyDown(Key::Backspace);
    field.frame();
    CHECK(field.text == "Has");
}

TEST_CASE("A click into the field with the focus keeps the cursor, which stays inside text that the game shortens") {
    TextFieldFrames field{
        .text = "Hasen",
    };
    field.frame();
    field.clickAt({60.0f, 5.0f});
    field.tap(Key::Home);
    field.clickAt({60.0f, 5.0f});
    field.type(u'x');
    CHECK(field.text == "xHasen");

    field.tap(Key::End);
    field.text = "Ha";
    field.type(u'!');
    CHECK(field.text == "Ha!");
}

TEST_CASE("Padding keeps the children away from the edge of their box, and a gap keeps them apart") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Signal column;
    Signal second;
    Signal share;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 1);
        column = ui.beginBox({
            .label = "Column",
            .padding = 4.0f,
            .gap = 2.0f,
        });
        ui.box({
            .label = "First",
            .width = SizeRule::pixels(10.0f),
            .height = SizeRule::pixels(10.0f),
        });
        second = ui.box({
            .label = "Second",
            .width = SizeRule::pixels(10.0f),
            .height = SizeRule::pixels(10.0f),
        });
        ui.endBox();
        ui.beginBox({
            .label = "Fixed",
            .width = SizeRule::pixels(40.0f),
            .height = SizeRule::pixels(20.0f),
            .padding = 5.0f,
        });
        share = ui.box({
            .label = "Share",
            .width = SizeRule::parentShare(1.0f),
            .height = SizeRule::parentShare(1.0f),
        });
        ui.endBox();
        ui.endFrame();
    }
    CHECK(column.rect.size == Vec2{18.0f, 30.0f});
    CHECK(second.rect.position == Vec2{4.0f, 16.0f});
    CHECK(share.rect.size == Vec2{30.0f, 10.0f});
}

TEST_CASE("A row puts its widgets side by side, with a gap between them") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Signal right;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 1);
        {
            const auto row = ui.row();
            ui.box({
                .label = "Left",
                .width = SizeRule::pixels(10.0f),
                .height = SizeRule::pixels(10.0f),
            });
            right = ui.box({
                .label = "Right",
                .width = SizeRule::pixels(10.0f),
                .height = SizeRule::pixels(10.0f),
            });
        }
        ui.endFrame();
    }
    CHECK(right.rect.position == Vec2{13.0f, 0.0f});
}

TEST_CASE("A panel floats at its position with its content below the title, and dragging the title moves it") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Signal inside;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        {
            const auto panel = ui.panel({
                .title = "Hasen",
                .position = {100.0f, 50.0f},
                .size = {200.0f, 100.0f},
            });
            inside = ui.box({
                .label = "Inside",
                .width = SizeRule::pixels(20.0f),
                .height = SizeRule::pixels(10.0f),
            });
        }
        ui.endFrame();
        input.beginFrame();
    };
    frame();
    frame();

    // The title takes 14 pixels, and the content keeps a padding of 4.
    CHECK(inside.rect.position == Vec2{104.0f, 68.0f});

    input.onMouseMove({150.0f, 55.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseMove({180.0f, 75.0f});
    frame();
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    CHECK(inside.rect.position == Vec2{134.0f, 88.0f});
}

TEST_CASE("The panel in front covers the panels behind it, and a click on a panel brings it to the front") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto area = [&] {
        const Signal signal = ui.box({
            .label = "Area",
            .width = SizeRule::parentShare(1.0f),
            .height = SizeRule::parentShare(1.0f),
            .clickable = true,
        });
        return signal.clicked;
    };
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        bool back = false;
        bool front = false;
        {
            const auto panel = ui.panel({
                .title = "Back",
                .position = {0.0f, 0.0f},
                .size = {100.0f, 100.0f},
            });
            back = area();
        }
        {
            const auto panel = ui.panel({
                .title = "Front",
                .position = {50.0f, 50.0f},
                .size = {100.0f, 100.0f},
            });
            front = area();
        }
        ui.endFrame();
        input.beginFrame();
        return std::pair{back, front};
    };
    const auto clickAt = [&](Vec2 position) {
        input.onMouseMove(position);
        input.onMouseButtonDown(MouseButton::Left);
        frame();
        input.onMouseButtonUp(MouseButton::Left);
        return frame();
    };
    frame();
    CHECK(clickAt({75.0f, 75.0f}) == std::pair{false, true});

    // The padding of the front panel lies over the area of the back panel and covers it.
    CHECK(clickAt({52.0f, 80.0f}) == std::pair{false, false});

    CHECK(clickAt({25.0f, 25.0f}) == std::pair{true, false});
    CHECK(clickAt({75.0f, 75.0f}) == std::pair{true, false});
}

TEST_CASE("A panel cuts off what does not fit, the wheel scrolls it into view, and clicks miss what is cut off") {
    constexpr etude::Color gray{
        .r = 0.5f,
        .g = 0.5f,
        .b = 0.5f,
    };
    Input input;
    Context ui{PixelFont{fontTexture}};
    std::array<Signal, 5> rows;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        {
            const auto panel = ui.panel({
                .title = "List",
                .position = {0.0f, 0.0f},
                .size = {100.0f, 60.0f},
            });
            for (std::size_t i = 0; i < rows.size(); ++i) {
                rows[i] = ui.box({
                    .label = std::format("Row {}", i),
                    .width = SizeRule::pixels(50.0f),
                    .height = SizeRule::pixels(20.0f),
                    .clickable = true,
                    .background = gray,
                });
            }
        }
        ui.endFrame();
        input.beginFrame();
    };
    const auto clickAt = [&](Vec2 position) {
        input.onMouseMove(position);
        input.onMouseButtonDown(MouseButton::Left);
        frame();
        input.onMouseButtonUp(MouseButton::Left);
        frame();
    };
    frame();
    frame();

    // Below the title, the content has 46 pixels for rows of 20 with gaps of 3, which start after a padding of 4.
    const etude::Rect content{
        .position = {0.0f, 14.0f},
        .size = {100.0f, 46.0f},
    };
    CHECK(std::ranges::any_of(ui.batches(), [&](const etude::DrawBatch& batch) { return batch.clip == content; }));
    clickAt({10.0f, 45.0f});
    CHECK(rows[1].clicked);

    input.onWheel(-1.0f);
    frame();
    clickAt({10.0f, 45.0f});
    CHECK(rows[2].clicked);
    clickAt({10.0f, 10.0f});
    CHECK_FALSE(rows[0].clicked);

    input.onWheel(-10.0f);
    frame();
    frame();
    CHECK(rows[4].rect.position.y == 36.0f);
}

TEST_CASE("A text field cuts off text that does not fit") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    std::string text(40, 'x');
    ui.beginFrame(input, screen, 1);
    ui.textField("Name", text);
    ui.endFrame();
    const etude::Rect field{
        .position = {30.0f, 0.0f},
        .size = {120.0f, 14.0f},
    };
    CHECK(std::ranges::any_of(ui.batches(), [&](const etude::DrawBatch& batch) { return batch.clip == field; }));
}

TEST_CASE("The UI wants the mouse over what it drew, and a press belongs to where it began until it ends") {
    constexpr etude::Color gray{
        .r = 0.5f,
        .g = 0.5f,
        .b = 0.5f,
    };
    Input input;
    Context ui{PixelFont{fontTexture}};
    bool drawn = true;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        if (drawn) {
            ui.box({
                .width = SizeRule::pixels(50.0f),
                .height = SizeRule::pixels(20.0f),
                .background = gray,
            });
        }

        // The second box draws nothing.
        ui.box({
            .width = SizeRule::pixels(50.0f),
            .height = SizeRule::pixels(20.0f),
        });
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

    // A box that the UI no longer draws lets go of the mouse after one frame.
    drawn = false;
    CHECK(frame());
    CHECK_FALSE(frame());
}

TEST_CASE("The UI does not want the mouse over what a panel cuts off") {
    constexpr etude::Color gray{
        .r = 0.5f,
        .g = 0.5f,
        .b = 0.5f,
    };
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        {
            const auto panel = ui.panel({
                .title = "List",
                .position = {0.0f, 0.0f},
                .size = {100.0f, 60.0f},
            });
            for (int i = 0; i < 3; ++i) {
                ui.box({
                    .width = SizeRule::pixels(50.0f),
                    .height = SizeRule::pixels(20.0f),
                    .background = gray,
                });
            }
        }
        ui.endFrame();
        const bool wanted = ui.wantsMouse();
        input.beginFrame();
        return wanted;
    };
    frame();

    // The panel ends at 60, and the third row starts at 64.
    input.onMouseMove({10.0f, 70.0f});
    CHECK_FALSE(frame());
    input.onMouseMove({10.0f, 50.0f});
    CHECK(frame());
}
