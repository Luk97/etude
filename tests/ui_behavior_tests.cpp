#include <catch2/catch_test_macros.hpp>

#include <etude/ui/box.h>
#include <etude/ui/column.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>

#include <array>
#include <cstddef>
#include <format>
#include <limits>
#include <utility>

using etude::Input;
using etude::Key;
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

    // Runs frames of a UI with a focusable field above a clickable button, like the game loop does, and tells whether
    // the field had the focus and whether the UI wanted the keyboard while the game ran.
    struct FocusFrames {
        Input input;
        Context ui{PixelFont{fontTexture}};

        std::pair<bool, bool> frame() {
            ui.beginFrame(input, screen, 1);
            const bool keyboard = ui.wantsKeyboard();
            const bool focused = box(ui, Modifier().key("Field").size(50.0f, 20.0f).focusable()).focused;
            box(ui, Modifier().key("Button").size(50.0f, 20.0f).clickable());
            ui.endFrame();
            input.beginFrame();
            return {focused, keyboard};
        }

        std::pair<bool, bool> clickAt(Vec2 position) {
            input.onMouseMove(position);
            input.onMouseButtonDown(MouseButton::Left);
            frame();
            input.onMouseButtonUp(MouseButton::Left);
            return frame();
        }
    };
}

TEST_CASE("A click gives a focusable element the focus, and a click elsewhere takes it away again") {
    FocusFrames frames;

    // The field lies at the top and the button below it, from y 20 on.
    CHECK(frames.frame() == std::pair{false, false});
    CHECK(frames.clickAt({10.0f, 10.0f}) == std::pair{true, true});
    CHECK(frames.clickAt({10.0f, 30.0f}) == std::pair{false, false});
    CHECK(frames.clickAt({10.0f, 10.0f}) == std::pair{true, true});
    CHECK(frames.clickAt({300.0f, 300.0f}) == std::pair{false, false});
}

TEST_CASE("Enter and Escape take the focus away at the end of their frame, so the game does not see them") {
    FocusFrames frames;
    frames.frame();
    for (const Key key : {Key::Enter, Key::Escape}) {
        frames.clickAt({10.0f, 10.0f});

        // In the frame of the key, the field keeps the focus and the UI the keyboard, one frame later both are gone.
        frames.input.onKeyDown(key);
        CHECK(frames.frame() == std::pair{true, true});
        frames.input.onKeyUp(key);
        CHECK(frames.frame() == std::pair{false, false});
    }
}

TEST_CASE("An element that a frame no longer builds loses the focus, so the keyboard returns to the game") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    bool shown = true;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        bool focused = false;
        if (shown) {
            focused = box(ui, Modifier().key("Field").size(50.0f, 20.0f).focusable()).focused;
        }
        ui.endFrame();
        input.beginFrame();
        return focused;
    };
    frame();
    input.onMouseMove({10.0f, 10.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseButtonUp(MouseButton::Left);
    CHECK(frame());
    CHECK(ui.wantsKeyboard());

    // A frame without the field ends its focus, which does not come back when the field shows again.
    shown = false;
    frame();
    CHECK_FALSE(ui.wantsKeyboard());
    shown = true;
    CHECK_FALSE(frame());
}

TEST_CASE("The cursor jumps past the end of any text when an element gains the focus, and stays while it keeps it") {
    FocusFrames frames;
    frames.frame();

    // A new focus puts the cursor past any text, a click into the field with the focus leaves it where it is.
    frames.clickAt({10.0f, 10.0f});
    CHECK(frames.ui.textCursor() == std::numeric_limits<std::size_t>::max());
    frames.ui.textCursor() = 2;
    frames.clickAt({10.0f, 10.0f});
    CHECK(frames.ui.textCursor() == 2);
}

TEST_CASE("The wheel scrolls the element under the mouse, as far as its children reach beyond its bottom") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    std::array<Interaction, 5> rows;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        column(ui, Modifier().key("List").size(100.0f, 40.0f).clip().verticalScroll(), [&] {
            for (std::size_t i = 0; i < rows.size(); ++i) {
                rows[i] = box(ui, Modifier().key(std::format("Row {}", i)).size(50.0f, 20.0f).background(gray));
            }
        });
        ui.endFrame();
        input.beginFrame();
    };

    // After two frames, the list knows how far it can scroll, and the mouse can find it. Every move of the wheel then
    // shows in the rectangles one frame later.
    frame();
    frame();

    // One notch moves the rows up by 24 pixels.
    input.onMouseMove({10.0f, 10.0f});
    input.onWheel(-1.0f);
    frame();
    frame();
    CHECK(rows[0].rect.position.y == -24.0f);

    // Five rows of 20 pixels reach 60 pixels beyond the bottom of the list, which is as far as it scrolls.
    input.onWheel(-10.0f);
    frame();
    frame();
    CHECK(rows[4].rect.position.y == 20.0f);

    // Away from the list, the wheel leaves it alone.
    input.onMouseMove({300.0f, 300.0f});
    input.onWheel(5.0f);
    frame();
    frame();
    CHECK(rows[4].rect.position.y == 20.0f);
}

TEST_CASE("When the content shrinks, the scrolling follows it in the same frame") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    std::array<Interaction, 5> rows;
    std::size_t count = rows.size();
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        column(ui, Modifier().key("List").size(100.0f, 40.0f).clip().verticalScroll(), [&] {
            for (std::size_t i = 0; i < count; ++i) {
                rows[i] = box(ui, Modifier().key(std::format("Row {}", i)).size(50.0f, 20.0f));
            }
        });
        ui.endFrame();
        input.beginFrame();
    };
    frame();
    frame();
    input.onMouseMove({10.0f, 10.0f});
    input.onWheel(-10.0f);
    frame();

    // Scrolled to the bottom, the list loses two rows, so it can only scroll by 20 pixels. The frame in which the rows
    // go already shows the last row at the bottom, which the next frame reports.
    count = 3;
    frame();
    frame();
    CHECK(rows[2].rect.position.y == 20.0f);
}

TEST_CASE("The wheel scrolls the innermost scrolling element under the mouse") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction top;
    Interaction below;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        column(ui, Modifier().key("Outer").size(100.0f, 60.0f).clip().verticalScroll(), [&] {
            column(ui, Modifier().key("Inner").size(100.0f, 40.0f).clip().verticalScroll(), [&] {
                top = box(ui, Modifier().key("Top").size(50.0f, 20.0f));
                box(ui, Modifier().key("Middle").size(50.0f, 20.0f));
                box(ui, Modifier().key("Bottom").size(50.0f, 20.0f));
            });
            below = box(ui, Modifier().key("Below").size(50.0f, 40.0f));
        });
        ui.endFrame();
        input.beginFrame();
    };
    frame();
    frame();

    // Both columns could scroll by 20 pixels, but only the inner one under the mouse does.
    input.onMouseMove({10.0f, 10.0f});
    input.onWheel(-1.0f);
    frame();
    frame();
    CHECK(top.rect.position.y == -20.0f);
    CHECK(below.rect.position.y == 40.0f);
}

TEST_CASE("Dragging a handle moves its floating element, which keeps the point where the mouse grabbed it") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction handle;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        box(ui, Modifier().key("Window").floating({100.0f, 50.0f}).size(80.0f, 40.0f).background(gray),
            [&] { handle = box(ui, Modifier().key("Handle").size(80.0f, 10.0f).dragHandle()); });
        ui.endFrame();
        input.beginFrame();
    };
    frame();
    frame();
    CHECK(handle.rect.position == Vec2{100.0f, 50.0f});

    // The mouse grabs the handle 10 by 5 pixels into it and moves 30 by 20 pixels, so the window follows by as much.
    input.onMouseMove({110.0f, 55.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseMove({140.0f, 75.0f});
    frame();
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    frame();
    CHECK(handle.rect.position == Vec2{130.0f, 70.0f});
}

TEST_CASE("A dragged handle stays on the screen, even when the mouse leaves the window") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction handle;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        box(ui, Modifier().key("Window").floating({100.0f, 50.0f}).size(80.0f, 40.0f).background(gray),
            [&] { handle = box(ui, Modifier().key("Handle").size(80.0f, 10.0f).dragHandle()); });
        ui.endFrame();
        input.beginFrame();
    };
    const auto drag = [&](Vec2 from, Vec2 to) {
        input.onMouseMove(from);
        input.onMouseButtonDown(MouseButton::Left);
        frame();
        input.onMouseMove(to);
        frame();
        input.onMouseButtonUp(MouseButton::Left);
        frame();
        frame();
    };
    frame();
    frame();

    // The handle is grabbed 10 by 5 pixels into it, and that point stops at the top left corner of the screen.
    drag({110.0f, 55.0f}, {-300.0f, -300.0f});
    CHECK(handle.rect.position == Vec2{-10.0f, -5.0f});

    // Grabbed there again, the same point stops at the last pixel of the screen on the other side.
    drag({0.0f, 0.0f}, {5000.0f, 5000.0f});
    CHECK(handle.rect.position == Vec2{629.0f, 354.0f});
}

TEST_CASE("The scale multiplies where a floating element starts and how far a notch of the wheel scrolls") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    std::array<Interaction, 5> rows;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 2);
        column(ui, Modifier().key("Window").floating({10.0f, 20.0f}).size(50.0f, 60.0f).clip().verticalScroll(), [&] {
            for (std::size_t i = 0; i < rows.size(); ++i) {
                rows[i] = box(ui, Modifier().key(std::format("Row {}", i)).size(50.0f, 20.0f));
            }
        });
        ui.endFrame();
        input.beginFrame();
    };
    frame();
    frame();
    CHECK(rows[0].rect.position == Vec2{20.0f, 40.0f});

    // At scale 2, one notch moves the rows up by 48 pixels.
    input.onMouseMove({30.0f, 50.0f});
    input.onWheel(-1.0f);
    frame();
    frame();
    CHECK(rows[0].rect.position.y == -8.0f);
}

TEST_CASE("A drag handle outside of floating elements moves nothing") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction handle;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        handle = box(ui, Modifier().key("Handle").size(80.0f, 10.0f).dragHandle());
        ui.endFrame();
        input.beginFrame();
    };
    frame();
    input.onMouseMove({10.0f, 5.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseMove({100.0f, 100.0f});
    frame();
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    frame();
    CHECK(handle.rect.position == Vec2{0.0f, 0.0f});
}
