#include <catch2/catch_test_macros.hpp>

#include <etude/platform/input.h>

using etude::Input;
using etude::Key;
using etude::MouseButton;

TEST_CASE("Input reports a key as pressed and held in the frame it goes down") {
    Input input;
    input.onKeyDown(Key::Space);
    CHECK(input.pressed(Key::Space));
    CHECK(input.held(Key::Space));
    CHECK_FALSE(input.released(Key::Space));
}

TEST_CASE("Input keeps a key held but no longer pressed in later frames") {
    Input input;
    input.onKeyDown(Key::Space);
    input.beginFrame();
    CHECK_FALSE(input.pressed(Key::Space));
    CHECK(input.held(Key::Space));
}

TEST_CASE("Input ignores repeated key-down messages of a held key") {
    Input input;
    input.onKeyDown(Key::A);
    input.beginFrame();
    input.onKeyDown(Key::A);
    CHECK_FALSE(input.pressed(Key::A));
    CHECK(input.held(Key::A));
}

TEST_CASE("Input reports a key as released only in the frame it goes up") {
    Input input;
    input.onKeyDown(Key::A);
    input.beginFrame();
    input.onKeyUp(Key::A);
    CHECK(input.released(Key::A));
    CHECK_FALSE(input.held(Key::A));
    input.beginFrame();
    CHECK_FALSE(input.released(Key::A));
}

TEST_CASE("Input keeps a tap that goes down and up within one frame") {
    Input input;
    input.onKeyDown(Key::Enter);
    input.onKeyUp(Key::Enter);
    CHECK(input.pressed(Key::Enter));
    CHECK(input.released(Key::Enter));
    CHECK_FALSE(input.held(Key::Enter));
}

TEST_CASE("Input releases all held keys and buttons when the window loses focus") {
    Input input;
    input.onKeyDown(Key::Left);
    input.onMouseButtonDown(MouseButton::Right);
    input.beginFrame();
    input.onFocusLost();
    CHECK(input.released(Key::Left));
    CHECK_FALSE(input.held(Key::Left));
    CHECK(input.released(MouseButton::Right));
    CHECK_FALSE(input.held(MouseButton::Right));
    CHECK_FALSE(input.released(Key::Right));
}

TEST_CASE("Input tracks mouse buttons and the cursor position") {
    Input input;
    input.onMouseMove({12.0f, 34.0f});
    input.onMouseButtonDown(MouseButton::Left);
    CHECK(input.pressed(MouseButton::Left));
    CHECK(input.held(MouseButton::Left));
    CHECK(input.mousePosition() == etude::Vec2{12.0f, 34.0f});
}

TEST_CASE("Keys and mouse buttons have readable names") {
    CHECK(etude::toString(Key::A) == "A");
    CHECK(etude::toString(Key::Digit7) == "7");
    CHECK(etude::toString(Key::Alt) == "Alt");
    CHECK(etude::toString(MouseButton::Middle) == "Middle");
}
