#pragma once

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
        Backspace,
        Delete,
        Tab,
        Home,
        End,
        Count
    };

    /// @brief Number of keys.
    inline constexpr std::size_t keyCount = static_cast<std::size_t>(Key::Count);

    /// @brief Returns the name of a key as it appears in log lines, for example "A", "7" or "Space".
    std::string_view toString(Key key);
}
