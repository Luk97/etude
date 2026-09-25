#pragma once

#include <etude/core/vec2.h>

#include <array>
#include <cstddef>
#include <string_view>

namespace etude {

    /// @brief A key on the keyboard. Letters and digits are contiguous, so window.cpp can map them by offset.
    enum class Key {
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        Digit0,
        Digit1,
        Digit2,
        Digit3,
        Digit4,
        Digit5,
        Digit6,
        Digit7,
        Digit8,
        Digit9,
        Space,
        Enter,
        Escape,
        Left,
        Right,
        Up,
        Down,
        Shift,
        Control,
        Alt,
        Count
    };

    /// @brief A button on the mouse.
    enum class MouseButton {
        Left,
        Right,
        Middle,
        Count
    };

    /// @brief Number of keys.
    inline constexpr std::size_t keyCount = static_cast<std::size_t>(Key::Count);

    /// @brief Number of mouse buttons.
    inline constexpr std::size_t mouseButtonCount = static_cast<std::size_t>(MouseButton::Count);

    /// @brief Returns the name of a key as it appears in logs lines, for example "A", "7" or "Space".
    std::string_view toString(Key key);

    /// @brief Returns the name of the mouse button, for example "Left".
    std::string_view toString(MouseButton button);

    class Input {
    public:
        bool pressed(Key key) const;
        bool held(Key key) const;
        bool released(Key key) const;

        bool pressed(MouseButton button) const;
        bool held(MouseButton button) const;
        bool released(MouseButton button) const;

        /// @brief Returns the cursor position in client pixels, measured from the top left corner of the window.
        Vec2 mousePosition() const;

        /// @brief Starts a new frame by forgetting which keys and buttons were pressed or released in the last one.
        void beginFrame();

        /// @brief Records that a key went down. Further calls while the key is held do not count as new presses.
        void onKeyDown(Key key);

        /// @brief Records that a key is no longer held down.
        void onKeyUp(Key key);

        /// @brief Records that a mouse button went down. Further calls while the button is held do not count as new
        /// presses.
        void onMouseButtonDown(MouseButton button);

        /// @brief Records that a mouse button is no longer held down.
        void onMouseButtonUp(MouseButton button);

        /// @brief Records that the cursor moved to a position in client pixels.
        void onMouseMove(Vec2 position);

        /// @brief Releases every held key and button. Without it, keys held while the window loses focus would stay
        /// down, because their key-up message goes to another window.
        void onFocusLost();

    private:
        /// @brief State of one key or mouse button.
        struct State {
            bool pressed = false;
            bool held = false;
            bool released = false;
        };

        static void goDown(State& state);
        static void goUp(State& state);

        std::array<State, keyCount> keys{};
        std::array<State, mouseButtonCount> mouseButtons{};
        Vec2 mouse;
    };
}
