#include <etude/platform/input.h>

#include <etude/core/utf8.h>

#include <cstddef>

namespace etude {

    bool Input::pressed(Key key) const {
        return keys[static_cast<std::size_t>(key)].pressed;
    }

    bool Input::held(Key key) const {
        return keys[static_cast<std::size_t>(key)].held;
    }

    bool Input::released(Key key) const {
        return keys[static_cast<std::size_t>(key)].released;
    }

    bool Input::repeated(Key key) const {
        return keys[static_cast<std::size_t>(key)].repeated;
    }

    bool Input::pressed(MouseButton button) const {
        return mouseButtons[static_cast<std::size_t>(button)].pressed;
    }

    bool Input::held(MouseButton button) const {
        return mouseButtons[static_cast<std::size_t>(button)].held;
    }

    bool Input::released(MouseButton button) const {
        return mouseButtons[static_cast<std::size_t>(button)].released;
    }

    Vec2 Input::mousePosition() const {
        return mouse;
    }

    Vec2 Input::mouseMovement() const {
        return mouse - frameStartMouse;
    }

    float Input::wheel() const {
        return wheelNotches;
    }

    std::string_view Input::text() const {
        return typed;
    }

    void Input::beginFrame() {
        // Pressed, released and repeated only last for one frame, while held stays until the key or button goes up.
        for (State& state : keys) {
            state.pressed = false;
            state.released = false;
            state.repeated = false;
        }
        for (State& state : mouseButtons) {
            state.pressed = false;
            state.released = false;
            state.repeated = false;
        }

        // The movement of the mouse counts from here, and the wheel and the typed text start anew.
        frameStartMouse = mouse;
        wheelNotches = 0.0f;
        typed.clear();
    }

    void Input::onKeyDown(Key key) {
        goDown(keys[static_cast<std::size_t>(key)]);
    }

    void Input::onKeyUp(Key key) {
        goUp(keys[static_cast<std::size_t>(key)]);
    }

    void Input::onMouseButtonDown(MouseButton button) {
        goDown(mouseButtons[static_cast<std::size_t>(button)]);
    }

    void Input::onMouseButtonUp(MouseButton button) {
        goUp(mouseButtons[static_cast<std::size_t>(button)]);
    }

    void Input::onMouseMove(Vec2 position) {
        mouse = position;
    }

    void Input::onWheel(float notches) {
        wheelNotches += notches;
    }

    void Input::onCharacter(char16_t unit) {
        if (unit >= 0xD800 && unit <= 0xDBFF) {
            highSurrogate = unit;
            return;
        }
        char32_t character = unit;
        if (unit >= 0xDC00 && unit <= 0xDFFF) {
            if (highSurrogate == 0) {
                return;
            }
            character = static_cast<char32_t>(0x10000 + ((highSurrogate - 0xD800) << 10) + (unit - 0xDC00));
        }
        highSurrogate = 0;
        if (character < 0x20 || character == 0x7F) {
            return;
        }
        appendUtf8(typed, character);
    }

    void Input::onFocusLost() {
        for (State& state : keys) {
            goUp(state);
        }
        for (State& state : mouseButtons) {
            goUp(state);
        }
    }

    void Input::goDown(State& state) {
        if (!state.held) {
            state.pressed = true;
            state.held = true;
        }
        state.repeated = true;
    }

    void Input::goUp(State& state) {
        if (state.held) {
            state.held = false;
            state.released = true;
        }
    }
}
