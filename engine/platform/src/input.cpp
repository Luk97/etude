#include <etude/platform/input.h>

namespace etude {

    constexpr auto keyNames = std::to_array<std::string_view>(
        {"A",     "B",     "C",      "D",    "E",     "F",  "G",    "H",     "I",       "J",  "K", "L",
         "M",     "N",     "O",      "P",    "Q",     "R",  "S",    "T",     "U",       "V",  "W", "X",
         "Y",     "Z",     "0",      "1",    "2",     "3",  "4",    "5",     "6",       "7",  "8", "9",
         "Space", "Enter", "Escape", "Left", "Right", "Up", "Down", "Shift", "Control", "Alt"}
    );
    static_assert(keyNames.size() == keyCount, "keyNames needs one name per Key, in the same order.");

    constexpr auto mouseButtonNames = std::to_array<std::string_view>({"Left", "Right", "Middle"});
    static_assert(mouseButtonNames.size() == mouseButtonCount, "mouseButtonNames needs one name per MouseButton.");

    std::string_view toString(Key key) {
        return keyNames[static_cast<std::size_t>(key)];
    }

    std::string_view toString(MouseButton button) {
        return mouseButtonNames[static_cast<std::size_t>(button)];
    }

    bool Input::pressed(Key key) const {
        return keys[static_cast<std::size_t>(key)].pressed;
    }

    bool Input::held(Key key) const {
        return keys[static_cast<std::size_t>(key)].held;
    }

    bool Input::released(Key key) const {
        return keys[static_cast<std::size_t>(key)].released;
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

    void Input::beginFrame() {
        for (State& state : keys) {
            state.pressed = false;
            state.released = false;
        }
        for (State& state : mouseButtons) {
            state.pressed = false;
            state.released = false;
        }
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
    }

    void Input::goUp(State& state) {
        if (state.held) {
            state.held = false;
            state.released = true;
        }
    }
}
