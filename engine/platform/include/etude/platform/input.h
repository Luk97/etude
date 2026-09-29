#pragma once

#include <etude/math/vec2.h>

#include <array>
#include <cstddef>
#include <string>
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
        Backspace,
        Delete,
        Tab,
        Home,
        End,
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

    /// @brief Returns the name of a key as it appears in log lines, for example "A", "7" or "Space".
    std::string_view toString(Key key);

    /// @brief Returns the name of the mouse button, for example "Left".
    std::string_view toString(MouseButton button);

    class Input {
    public:
        bool pressed(Key key) const;
        bool held(Key key) const;
        bool released(Key key) const;
        /// @brief Returns whether the key went down in this frame or repeated while held, as the keyboard repeats keys
        /// that stay down. Text fields react to it, while a game usually wants pressed.
        bool repeated(Key key) const;

        bool pressed(MouseButton button) const;
        bool held(MouseButton button) const;
        bool released(MouseButton button) const;

        /// @brief Returns the cursor position in client pixels, measured from the top left corner of the window.
        Vec2 mousePosition() const;

        /// @brief Returns how far the mouse wheel turned in this frame, in notches. Positive values turn it away from
        /// the user, which usually scrolls up.
        float wheel() const;

        /// @brief Returns the text typed in this frame as UTF-8, without control characters such as backspace, which
        /// text fields read as keys instead.
        std::string_view text() const;

        /// @brief Starts a new frame by forgetting which keys and buttons were pressed or released in the last one,
        /// how far the wheel turned and what was typed.
        void beginFrame();

        /// @brief Records that a key went down. Further calls while the key is held do not count as new presses, only
        /// as repeats.
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

        /// @brief Records that the mouse wheel turned by the given notches.
        void onWheel(float notches);

        /// @brief Records a UTF-16 code unit of typed text, as WM_CHAR delivers it. A character outside the basic
        /// plane arrives as a pair of surrogates, which becomes one character of the text.
        void onCharacter(char16_t unit);

        /// @brief Releases every held key and button. Without it, keys held while the window loses focus would stay
        /// down, because their key-up message goes to another window.
        void onFocusLost();

    private:
        /// @brief State of one key or mouse button.
        struct State {
            bool pressed = false;
            bool held = false;
            bool released = false;
            bool repeated = false;
        };

        static void goDown(State& state);
        static void goUp(State& state);

        std::array<State, keyCount> keys{};
        std::array<State, mouseButtonCount> mouseButtons{};
        Vec2 mouse;
        float wheelNotches = 0.0f;
        std::string typed;

        /// @brief The first half of a surrogate pair, until the second half arrives.
        char16_t highSurrogate = 0;
    };
}
