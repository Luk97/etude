#include <catch2/catch_test_macros.hpp>

#include <etude/ui/alignment.h>
#include <etude/ui/box.h>
#include <etude/ui/column.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>
#include <etude/ui/row.h>
#include <etude/ui/text.h>

#include <utility>

using etude::Input;
using etude::PixelFont;
using etude::Vec2;
using etude::ui::Alignment;
using etude::ui::Context;
using etude::ui::Interaction;
using etude::ui::Modifier;

namespace {

    constexpr auto fontTexture = static_cast<etude::TextureId>(7);
    constexpr etude::Size screen{640, 360};
}

TEST_CASE("Elements line up from the top of the screen, and the next frame tells where they were") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        const Interaction first = box(ui, Modifier().key("First").size(50.0f, 20.0f));
        const Interaction second = box(ui, Modifier().key("Second").size(30.0f, 10.0f));
        ui.endFrame();
        return std::pair{first, second};
    };

    // The first frame knows no rectangles yet, the second one reports those of the first.
    CHECK(frame().first.rect.size == Vec2{});
    const auto [first, second] = frame();
    CHECK(first.rect.position == Vec2{0.0f, 0.0f});
    CHECK(first.rect.size == Vec2{50.0f, 20.0f});
    CHECK(second.rect.position == Vec2{0.0f, 20.0f});
    CHECK(second.rect.size == Vec2{30.0f, 10.0f});
}

TEST_CASE("A row adds up its children from left to right and is as high as the highest of them") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction outer;
    Interaction right;

    // Two frames, since an interaction reports the rectangle of the last frame.
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 1);
        outer = row(ui, Modifier().key("Row"), [&] {
            box(ui, Modifier().key("Left").size(50.0f, 20.0f));
            right = box(ui, Modifier().key("Right").size(30.0f, 40.0f));
        });
        ui.endFrame();
    }
    CHECK(outer.rect.size == Vec2{80.0f, 40.0f});
    CHECK(right.rect.position == Vec2{50.0f, 0.0f});
}

TEST_CASE("A column adds up its children from top to bottom and is as wide as the widest of them") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction outer;
    Interaction lower;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 1);
        outer = column(ui, Modifier().key("Column"), [&] {
            box(ui, Modifier().key("Upper").size(50.0f, 20.0f));
            lower = box(ui, Modifier().key("Lower").size(30.0f, 40.0f));
        });
        ui.endFrame();
    }
    CHECK(outer.rect.size == Vec2{50.0f, 60.0f});
    CHECK(lower.rect.position == Vec2{0.0f, 20.0f});
}

TEST_CASE("A box lays its children on top of each other and is as large as the largest of them") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction stack;
    Interaction first;
    Interaction second;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 1);
        stack = box(ui, Modifier().key("Stack"), [&] {
            first = box(ui, Modifier().key("First").size(50.0f, 20.0f));
            second = box(ui, Modifier().key("Second").size(30.0f, 40.0f));
        });
        ui.endFrame();
    }
    CHECK(stack.rect.size == Vec2{50.0f, 40.0f});
    CHECK(first.rect.position == Vec2{0.0f, 0.0f});
    CHECK(second.rect.position == Vec2{0.0f, 0.0f});
}

TEST_CASE("The alignment puts the children into the middle or at the end of the space that they leave free") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction centered;
    Interaction ended;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 1);
        box(ui, Modifier().key("Box").size(100.0f, 50.0f).contentAlignment(Alignment::Center),
            [&] { centered = box(ui, Modifier().key("Child").size(20.0f, 10.0f)); });

        // A row only aligns from top to bottom, from left to right its children follow each other.
        row(ui, Modifier().key("Row").size(100.0f, 50.0f).contentAlignment(Alignment::End),
            [&] { ended = box(ui, Modifier().key("Child").size(20.0f, 10.0f)); });
        ui.endFrame();
    }

    // The box lies at the top, so its child sits at (100 - 20) / 2 and (50 - 10) / 2. The row starts below it.
    CHECK(centered.rect.position == Vec2{40.0f, 20.0f});
    CHECK(ended.rect.position == Vec2{0.0f, 90.0f});
}

TEST_CASE("A share of the parent takes that part of the space inside its padding") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction child;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 1);
        box(ui, Modifier().key("Parent").size(200.0f, 100.0f),
            [&] { child = box(ui, Modifier().key("Child").fillMaxWidth(0.5f).fillMaxHeight(0.25f)); });
        ui.endFrame();
    }
    CHECK(child.rect.size == Vec2{100.0f, 25.0f});
}

TEST_CASE("A text is as large as its line plus the padding, and the scale multiplies text, padding and pixels") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction word;
    Interaction square;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 2);
        word = text(ui, "Hasen", Modifier().key("Hasen").padding(4.0f, 2.0f));
        square = box(ui, Modifier().key("Square").size(10.0f, 10.0f));
        ui.endFrame();
    }

    // At scale 2, five characters of 6 by 8 pixels cover 60 by 16 pixels, and the padding adds 8 and 4 on each side.
    CHECK(word.rect.size == Vec2{76.0f, 24.0f});
    CHECK(square.rect.size == Vec2{20.0f, 20.0f});
}

TEST_CASE("A text keeps the padding of a line, unless the modifier sets another") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction line;
    Interaction bare;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 1);
        line = text(ui, "OK", Modifier().key("Line"));
        bare = text(ui, "OK", Modifier().key("Bare").padding(0.0f));
        ui.endFrame();
    }

    // Two characters cover 12 by 8 pixels, and a line adds 3 pixels on each side.
    CHECK(line.rect.size == Vec2{18.0f, 14.0f});
    CHECK(bare.rect.size == Vec2{12.0f, 8.0f});
}

TEST_CASE("Padding keeps the children away from the edge of their layout, and spacing keeps them apart") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction outer;
    Interaction second;
    Interaction share;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 1);
        outer = column(ui, Modifier().key("Column").padding(4.0f).spacedBy(2.0f), [&] {
            box(ui, Modifier().key("First").size(10.0f, 10.0f));
            second = box(ui, Modifier().key("Second").size(10.0f, 10.0f));
        });
        box(ui, Modifier().key("Fixed").size(40.0f, 20.0f).padding(5.0f),
            [&] { share = box(ui, Modifier().key("Share").fillMaxSize()); });
        ui.endFrame();
    }

    // The column takes 10 by 10 + 2 + 10 pixels and 4 more on each side. The share fills the 30 by 10 pixels inside the
    // padding of 5.
    CHECK(outer.rect.size == Vec2{18.0f, 30.0f});
    CHECK(second.rect.position == Vec2{4.0f, 16.0f});
    CHECK(share.rect.size == Vec2{30.0f, 10.0f});
}

TEST_CASE("A row keeps its spacing between its children") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    Interaction right;
    for (int frame = 0; frame < 2; ++frame) {
        ui.beginFrame(input, screen, 1);
        row(ui, Modifier().spacedBy(3.0f), [&] {
            box(ui, Modifier().key("Left").size(10.0f, 10.0f));
            right = box(ui, Modifier().key("Right").size(10.0f, 10.0f));
        });
        ui.endFrame();
    }
    CHECK(right.rect.position == Vec2{13.0f, 0.0f});
}
