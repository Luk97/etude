#pragma once

#include <etude/math/mat3.h>
#include <etude/math/rect.h>
#include <etude/math/size.h>
#include <etude/math/vec2.h>

namespace etude {

    /// @brief Decides which part of the world the window shows. The position lies in the top left corner of the
    /// window, and at zoom 1 one unit of the world covers one pixel.
    struct Camera2D {
        Vec2 position;
        float zoom = 1.0f;

        /// @brief Returns the matrix that maps world coordinates to clip space for a window of the given size.
        constexpr Mat3 viewProjection(Size window) const {
            const Rect view{
                .position = position,
                .size = {static_cast<float>(window.width) / zoom, static_cast<float>(window.height) / zoom},
            };
            return Mat3::orthographic(view);
        }
    };
}
