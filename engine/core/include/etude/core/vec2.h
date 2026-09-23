#pragma once

#include <cmath>

namespace etude {

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

        constexpr float lengthSquared() const {
            return x * x + y * y;
        }

        float length() const {
            return std::sqrt(lengthSquared());
        }
    };
}
