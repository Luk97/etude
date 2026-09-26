#pragma once

#include <cmath>

namespace etude {

    /// @brief Two-dimensional vector of floats for positions, directions and sizes.
    struct Vec2 {
        float x = 0.0f;
        float y = 0.0f;

        constexpr Vec2 operator+(Vec2 other) const {
            return {x + other.x, y + other.y};
        }

        constexpr Vec2 operator-(Vec2 other) const {
            return {x - other.x, y - other.y};
        }

        constexpr Vec2 operator*(float scalar) const {
            return {x * scalar, y * scalar};
        }

        constexpr bool operator==(const Vec2&) const = default;

        /// @brief Returns the squared length. Cheaper than length(), because it skips the square root,
        /// which is enough for comparing lengths.
        constexpr float lengthSquared() const {
            return x * x + y * y;
        }

        float length() const {
            return std::sqrt(lengthSquared());
        }
    };
}
