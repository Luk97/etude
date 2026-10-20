#pragma once

#include <etude/math/vec2.h>
#include <etude/platform/key.h>
#include <etude/platform/mouse_button.h>

#include <array>
#include <string>
#include <string_view>

namespace etude {

    /// @brief The keyboard and the mouse as the window reports them: which keys and buttons are held, which went down
    /// or up in this frame, where the cursor is, how far the wheel turned and what was typed.
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

        /// @brief Returns how far the cursor moved in this frame, in client pixels.
        Vec2 mouseMovement() const;

        /// @brief Returns how far the mouse wheel turned in this frame, in notches. Positive values turn it away from
        /// the user, which usually scrolls up.
        float wheel() const;

        /// @brief Returns the text typed in this frame as UTF-8, without control characters such as backspace, which
        /// text fields read as keys instead.
        std::string_view text() const;

        /// @brief Starts a new frame by forgetting which keys and buttons were pressed or released in the last one,
        /// how far the wheel turned and what was typed. The mouse moves from where it is now.
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

        /// @brief Where the mouse was when the frame began.
        Vec2 frameStartMouse;

        float wheelNotches = 0.0f;
        std::string typed;

        /// @brief The first half of a surrogate pair, until the second half arrives.
        char16_t highSurrogate = 0;
    };
}
