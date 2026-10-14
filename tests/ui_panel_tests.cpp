#include <catch2/catch_test_macros.hpp>

#include <etude/ui/box.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>
#include <etude/ui/widgets/panel.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
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

TEST_CASE("A panel floats at its position with its content below the title, and dragging the title moves it") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction inside;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        panel(ui, "Hasen", {100.0f, 50.0f}, {200.0f, 100.0f}, [&] {
            inside = box(ui, Modifier().key("Inside").size(20.0f, 10.0f));
        });
        ui.endFrame();
        input.beginFrame();
    };
    frame();
    frame();

    // The title takes 14 pixels, and the content keeps a padding of 4.
    CHECK(inside.rect.position == Vec2{104.0f, 68.0f});

    // Grabbed at the title, the panel follows the mouse by 30 by 20 pixels, and its content with it.
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
    const auto area = [&] { return box(ui, Modifier().key("Area").fillMaxSize().clickable()).clicked; };
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        bool back = false;
        bool front = false;
        panel(ui, "Back", {0.0f, 0.0f}, {100.0f, 100.0f}, [&] { back = area(); });
        panel(ui, "Front", {50.0f, 50.0f}, {100.0f, 100.0f}, [&] { front = area(); });
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

    // The panels overlap from 50 to 100, and there the one in front takes the click.
    CHECK(clickAt({75.0f, 75.0f}) == std::pair{false, true});

    // The padding of the front panel lies over the area of the back panel and covers it.
    CHECK(clickAt({52.0f, 80.0f}) == std::pair{false, false});

    // A click on the back panel brings it to the front, so from then on it takes the clicks where they overlap.
    CHECK(clickAt({25.0f, 25.0f}) == std::pair{true, false});
    CHECK(clickAt({75.0f, 75.0f}) == std::pair{true, false});
}

TEST_CASE("The panel in front is drawn last, so that it covers the panels behind it") {
    constexpr etude::Color red{
        .r = 1.0f,
    };
    constexpr etude::Color blue{
        .b = 1.0f,
    };
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto square = [&](etude::Color color) { box(ui, Modifier().size(10.0f, 10.0f).background(color)); };
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        panel(ui, "Back", {0.0f, 0.0f}, {100.0f, 100.0f}, [&] { square(red); });
        panel(ui, "Front", {50.0f, 50.0f}, {100.0f, 100.0f}, [&] { square(blue); });
        ui.endFrame();
        input.beginFrame();
    };

    // Whether the square of the back panel comes before that of the front panel among the sprites.
    const auto redFirst = [&] {
        const auto sprites = ui.sprites();
        const auto isRed = [](const etude::Sprite& sprite) { return sprite.color.r == 1.0f; };
        const auto isBlue = [](const etude::Sprite& sprite) { return sprite.color.b == 1.0f; };
        return std::ranges::find_if(sprites, isRed) < std::ranges::find_if(sprites, isBlue);
    };
    frame();
    CHECK(redFirst());

    // A click on the back panel outside the overlap brings it to the front, so it is drawn last from then on.
    input.onMouseMove({10.0f, 30.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    CHECK_FALSE(redFirst());
}

TEST_CASE("A panel cuts off what does not fit, the wheel scrolls it into view, and clicks miss what is cut off") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    std::array<Interaction, 5> rows;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        panel(ui, "List", {0.0f, 0.0f}, {100.0f, 60.0f}, [&] {
            for (std::size_t i = 0; i < rows.size(); ++i) {
                const Modifier row = Modifier().key(std::format("Row {}", i)).size(50.0f, 20.0f);
                rows[i] = box(ui, row.background(gray).clickable());
            }
        });
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

    // One notch moves the rows up by 24 pixels, so the third row lies at y 45, and the first one hides under the title.
    input.onWheel(-1.0f);
    frame();
    clickAt({10.0f, 45.0f});
    CHECK(rows[2].clicked);
    clickAt({10.0f, 10.0f});
    CHECK_FALSE(rows[0].clicked);

    // The wheel only scrolls while the mouse is over the content, not over the title.
    input.onWheel(-10.0f);
    frame();
    frame();
    CHECK(rows[4].rect.position.y == 86.0f);
    input.onMouseMove({10.0f, 45.0f});
    input.onWheel(-10.0f);
    frame();
    frame();
    CHECK(rows[4].rect.position.y == 36.0f);
}

TEST_CASE("The UI does not want the mouse over what a panel cuts off") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        panel(ui, "List", {0.0f, 0.0f}, {100.0f, 60.0f}, [&] {
            for (int i = 0; i < 3; ++i) {
                box(ui, Modifier().size(50.0f, 20.0f).background(gray));
            }
        });
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

TEST_CASE("At scale 2, a panel and its content keep their proportions") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction inside;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 2);
        panel(ui, "Hasen", {100.0f, 50.0f}, {200.0f, 100.0f}, [&] {
            inside = box(ui, Modifier().key("Inside").size(20.0f, 10.0f));
        });
        ui.endFrame();
    }

    // Position, title, padding and size all count twice: the content starts at (100 + 4) * 2 and (50 + 14 + 4) * 2.
    CHECK(inside.rect.position == Vec2{208.0f, 136.0f});
    CHECK(inside.rect.size == Vec2{40.0f, 20.0f});
}

TEST_CASE("A panel tells whether its title is held, and comes back where it was after the game hid it") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    bool shown = true;
    Interaction title;
    Interaction inside;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        if (shown) {
            title = panel(ui, "Hasen", {100.0f, 50.0f}, {200.0f, 100.0f}, [&] {
                inside = box(ui, Modifier().key("Inside").size(20.0f, 10.0f));
            });
        }
        ui.endFrame();
        input.beginFrame();
    };
    frame();
    frame();

    // While the title is held, the panel follows the mouse by 30 by 20 pixels.
    input.onMouseMove({150.0f, 55.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    CHECK(title.held);
    input.onMouseMove({180.0f, 75.0f});
    frame();
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    CHECK_FALSE(title.held);

    // Hidden for two frames and shown again, the panel keeps the place it was dragged to.
    shown = false;
    frame();
    frame();
    shown = true;
    frame();
    frame();
    CHECK(inside.rect.position == Vec2{134.0f, 88.0f});
}
