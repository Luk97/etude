#pragma once

#include <numbers>

namespace etude {

    /// @brief Converts an angle from degrees, in which Transform2D and the editor keep it, into radians, which sprites
    /// and the functions of <cmath> take.
    constexpr float toRadians(float degrees) {
        return degrees * (std::numbers::pi_v<float> / 180.0f);
    }
}
