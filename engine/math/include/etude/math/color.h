#pragma once

#include <cmath>

namespace etude {

    /// @brief A color with red, green, blue and alpha, each from 0 to 1. Red, green and blue are in sRGB, as image
    /// editors and color pickers show them, while alpha covers linearly.
    struct Color {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 1.0f;
    };

    /// @brief Converts the color from sRGB into linear light, in which the GPU blends. Alpha stays as it is.
    inline Color toLinear(Color color) {
        const auto linear = [](float channel) {
            return channel <= 0.04045f ? channel / 12.92f : std::pow((channel + 0.055f) / 1.055f, 2.4f);
        };
        return {
            .r = linear(color.r),
            .g = linear(color.g),
            .b = linear(color.b),
            .a = color.a,
        };
    }
}
