#include <catch2/catch_test_macros.hpp>

#include <etude/ui/context.h>
#include <etude/ui/row.h>
#include <etude/ui/text.h>
#include <etude/ui/widgets/button.h>
#include <etude/ui/widgets/checkbox.h>
#include <etude/ui/widgets/slider.h>
#include <etude/ui/widgets/text_field.h>

#include <algorithm>
#include <string>
#include <utility>

using etude::Input;
using etude::Key;
using etude::MouseButton;
using etude::PixelFont;
using etude::Vec2;
using etude::ui::Context;
using etude::ui::Interaction;

namespace {

    constexpr auto fontTexture = static_cast<etude::TextureId>(7);
    constexpr etude::Size screen{640, 360};

    /// @brief Counts the sprites of the last frame that show a character.
    auto countGlyphs(const Context& ui) {
        return std::ranges::count_if(ui.sprites(), [](const etude::Sprite& sprite) {
            return sprite.texture == fontTexture;
        });
    }

    // Runs frames of a UI with one text field, like the game loop does. Its label Name takes 30 pixels, so the field
    // starts at x 30.
    struct TextFieldFrames {
        Input input;
        Context ui{PixelFont{fontTexture}};
        std::string text;

        Interaction frame() {
            ui.beginFrame(input, screen, 1);
            const Interaction field = textField(ui, "Name", text);
            ui.endFrame();
            input.beginFrame();
            return field;
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

        Interaction type(char16_t character) {
            input.onCharacter(character);
            return frame();
        }
    };
}

TEST_CASE("A button counts a click when the left button comes up over it") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        const bool clicked = button(ui, "OK").clicked;
        ui.endFrame();
        input.beginFrame();
        return clicked;
    };

    // Neither the mouse over the button nor the press counts, only the release over it, and only in its frame.
    input.onMouseMove({5.0f, 5.0f});
    CHECK_FALSE(frame());
    input.onMouseButtonDown(MouseButton::Left);
    CHECK_FALSE(frame());
    input.onMouseButtonUp(MouseButton::Left);
    CHECK(frame());
    CHECK_FALSE(frame());
}

TEST_CASE("A button draws its background and then its text in the middle") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    ui.beginFrame(input, screen, 1);
    button(ui, "OK");
    ui.endFrame();

    // The background is 12 + 2 * 6 by 8 + 2 * 3 pixels, and the two characters sit inside its padding.
    const auto sprites = ui.sprites();
    REQUIRE(sprites.size() == 3);
    CHECK(sprites[0].position == Vec2{0.0f, 0.0f});
    CHECK(sprites[0].size == Vec2{24.0f, 14.0f});
    CHECK(sprites[0].texture == etude::TextureId{});
    CHECK(sprites[1].position == Vec2{6.0f, 3.0f});
    CHECK(sprites[1].texture == fontTexture);
    CHECK(sprites[2].position == Vec2{12.0f, 3.0f});
}

TEST_CASE("Buttons whose labels only differ after ## look alike but stay apart") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        bool first = false;
        bool second = false;
        row(ui, [&] {
            first = button(ui, "OK").clicked;
            second = button(ui, "OK##2").clicked;
        });
        ui.endFrame();
        input.beginFrame();
        return std::pair{first, second};
    };
    frame();

    // Both buttons show OK, but the first one is only 24 pixels wide, so the click at x 30 hits the second.
    CHECK(countGlyphs(ui) == 4);
    input.onMouseMove({30.0f, 5.0f});
    input.onMouseButtonDown(MouseButton::Left);
    frame();
    input.onMouseButtonUp(MouseButton::Left);
    CHECK(frame() == std::pair{false, true});
}

TEST_CASE("Texts and text fields show ## in their text as it is") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    std::string value = "x##y";
    ui.beginFrame(input, screen, 1);
    text(ui, "a##b");
    textField(ui, "Name", value);
    ui.endFrame();

    // a##b and x##y have four characters each, and the label Name four more.
    CHECK(countGlyphs(ui) == 12);
}

TEST_CASE("A checkbox flips its value when a click on the square or on its text ends") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    bool value = false;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        const bool clicked = checkbox(ui, "Pause", value).clicked;
        ui.endFrame();
        input.beginFrame();
        return clicked;
    };
    const auto clickAt = [&](Vec2 position) {
        input.onMouseMove(position);
        input.onMouseButtonDown(MouseButton::Left);
        frame();
        input.onMouseButtonUp(MouseButton::Left);
        return frame();
    };
    frame();

    // The press on the square flips nothing yet, the release over it does.
    input.onMouseMove({5.0f, 5.0f});
    input.onMouseButtonDown(MouseButton::Left);
    CHECK_FALSE(frame());
    CHECK_FALSE(value);
    input.onMouseButtonUp(MouseButton::Left);
    CHECK(frame());
    CHECK(value);

    // The text beside the square takes the click as well.
    CHECK(clickAt({30.0f, 5.0f}));
    CHECK_FALSE(value);
}

TEST_CASE("A slider takes the value where the mouse presses or drags it, within its range, while it is held") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    float value = 0.0f;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        const bool held = slider(ui, "Tempo", value, 0.0f, 2.0f).held;
        ui.endFrame();
        input.beginFrame();
        return held;
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

TEST_CASE("A slider also takes the value of a press that comes up again within the same frame") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    float value = 0.0f;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        slider(ui, "Tempo", value, 0.0f, 2.0f);
        ui.endFrame();
        input.beginFrame();
    };
    frame();

    // The bar runs from x 36 to 136, so a tap at x 111 lies three quarters along it.
    input.onMouseMove({111.0f, 5.0f});
    input.onMouseButtonDown(MouseButton::Left);
    input.onMouseButtonUp(MouseButton::Left);
    frame();
    CHECK(value == 1.5f);
}

TEST_CASE("A text field takes typed text from a press on it until Enter, Escape or a press elsewhere") {
    TextFieldFrames field{
        .text = "Hase",
    };
    field.frame();

    // Without the focus, typing does nothing. A click gives the field the focus, and Enter ends the typing.
    CHECK_FALSE(field.type(u'x').focused);
    field.clickAt({60.0f, 5.0f});
    CHECK(field.type(u'n').focused);
    field.tap(Key::Enter);
    CHECK_FALSE(field.type(u'x').focused);
    CHECK(field.text == "Hasen");

    // Escape and a click elsewhere end the typing as well.
    field.clickAt({60.0f, 5.0f});
    field.tap(Key::Escape);
    field.type(u'x');
    CHECK(field.text == "Hasen");

    field.clickAt({60.0f, 5.0f});
    field.clickAt({400.0f, 300.0f});
    field.type(u'x');
    CHECK(field.text == "Hasen");
}

TEST_CASE("Backspace, Delete, the left and right arrow keys, Home and End edit the text character by character") {
    TextFieldFrames field{
        .text = "aä€",
    };
    field.frame();

    // The cursor starts at the end, and every key moves over or removes a whole character of one to three bytes.
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

    // The press removes a character, a frame without a repeat of the keyboard none, and every repeat one more. Like
    // Windows, the input gets a repeat as another key down without a key up in between.
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

    // Home puts the cursor at the start, and a second click into the field leaves it there.
    field.clickAt({60.0f, 5.0f});
    field.tap(Key::Home);
    field.clickAt({60.0f, 5.0f});
    field.type(u'x');
    CHECK(field.text == "xHasen");

    // When the game shortens the text, the cursor falls back to its end.
    field.tap(Key::End);
    field.text = "Ha";
    field.type(u'!');
    CHECK(field.text == "Ha!");
}

TEST_CASE("The cursor moves to the start of a character when the game changes the text around it") {
    TextFieldFrames field{
        .text = "a",
    };
    field.frame();

    // The cursor stands after the a, at byte 1, which lies inside the two bytes of the ä that the game puts there.
    field.clickAt({60.0f, 5.0f});
    field.text = "ä";
    field.type(u'b');
    CHECK(field.text == "bä");
}

TEST_CASE("A text field cuts off text that does not fit") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    std::string value(40, 'x');
    ui.beginFrame(input, screen, 1);
    textField(ui, "Name", value);
    ui.endFrame();

    // The label Name takes 30 pixels, and the field of 120 by 14 pixels after it clips its text.
    const etude::Rect field{
        .position = {30.0f, 0.0f},
        .size = {120.0f, 14.0f},
    };
    CHECK(std::ranges::any_of(ui.batches(), [&](const etude::DrawBatch& batch) { return batch.clip == field; }));
}
