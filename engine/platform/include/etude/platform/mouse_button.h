#pragma once

#include <cstddef>
#include <string_view>

namespace etude {

    /// @brief A button on the mouse.
    enum class MouseButton {
        Left,
        Right,
        Middle,
        Count
    };

    /// @brief Number of mouse buttons.
    inline constexpr std::size_t mouseButtonCount = static_cast<std::size_t>(MouseButton::Count);

    /// @brief Returns the name of the mouse button, for example "Left".
    std::string_view toString(MouseButton button);
}
