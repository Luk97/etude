#pragma once

#include <cstdint>
#include <vector>

namespace etude {

    /// @brief One pixel with 8 bits each for red, green, blue and alpha, in this order. Starts as transparent black,
    /// the value of zeroed memory.
    struct Pixel {
        std::uint8_t r = 0;
        std::uint8_t g = 0;
        std::uint8_t b = 0;
        std::uint8_t a = 0;

        constexpr bool operator==(const Pixel&) const = default;
    };

    /// @brief An image in memory with width times height pixels, row by row from the top, each row from left to right.
    struct Image {
        int width = 0;
        int height = 0;
        std::vector<Pixel> pixels;

        bool operator==(const Image&) const = default;
    };
}
