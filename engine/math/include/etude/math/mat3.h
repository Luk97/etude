#pragma once

#include <etude/math/rect.h>
#include <etude/math/vec2.h>

#include <array>
#include <cmath>
#include <cstddef>

namespace etude {

    /// @brief A 3x3 matrix for affine 2D transfroms such as translation, rotation and scaling. The columns lie one
    /// after another in memory like in GLSL, the third column hold sthe translation and the bottom row stays 0, 0, 1.
    struct Mat3 {
        std::array<std::array<float, 3>, 3> columns{{{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

        /// @brief Moves points by the offset.
        static constexpr Mat3 translation(Vec2 offset) {
            Mat3 matrix;
            matrix.columns[2][0] = offset.x;
            matrix.columns[2][1] = offset.y;
            return matrix;
        }

        /// @brief Scales points away from the origin, separately along x and y.
        static constexpr Mat3 scaling(Vec2 factors) {
            Mat3 matrix;
            matrix.columns[0][0] = factors.x;
            matrix.columns[1][1] = factors.y;
            return matrix;
        }

        /// @brief Turns points around the origin. Because y points down, positive angles turn clockwise on the screen.
        static Mat3 rotation(float radians) {
            const float cosine = std::cos(radians);
            const float sine = std::sin(radians);
            Mat3 matrix;
            matrix.columns[0][0] = cosine;
            matrix.columns[0][1] = sine;
            matrix.columns[1][0] = -sine;
            matrix.columns[1][1] = cosine;
            return matrix;
        }

        /// @brief Maps the view rectangle onto the clip space of the GPU, from -1 at its top-left to 1 at its
        /// bottom-right corner. The clip space of Vulkan points y down like the world, so nothing is flipped.
        static constexpr Mat3 orthographic(Rect view) {
            const Vec2 scale{2.0f / view.size.x, 2.0f / view.size.y};
            Mat3 matrix = scaling(scale);
            matrix.columns[2][0] = -1.0f - view.position.x * scale.x;
            matrix.columns[2][1] = -1.0f - view.position.y * scale.y;
            return matrix;
        }

        /// @brief Combines two transforms into one. The right one applies first.
        constexpr Mat3 operator*(const Mat3& other) const {
            Mat3 product;
            for (std::size_t column = 0; column < 3; ++column) {
                for (std::size_t row = 0; row < 3; ++row) {
                    float sum = 0.0f;
                    for (std::size_t i = 0; i < 3; ++i) {
                        sum += columns[i][row] * other.columns[column][i];
                    }
                    product.columns[column][row] = sum;
                }
            }
            return product;
        }

        /// @brief Transforms a point, so the translation applies as well.
        constexpr Vec2 operator*(Vec2 point) const {
            return {
                .x = columns[0][0] * point.x + columns[1][0] * point.y + columns[2][0],
                .y = columns[0][1] * point.x + columns[1][1] * point.y + columns[2][1],
            };
        }

        constexpr bool operator==(const Mat3&) const = default;
    };
}
