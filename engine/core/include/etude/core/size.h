#pragma once

namespace etude {

    /// @brief Width and height in whole pixels, for example of the client area of a window.
    struct Size {
        int width = 0;
        int height = 0;

        constexpr bool operator==(const Size&) const = default;
    };
}
