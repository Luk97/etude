#pragma once

#include <etude/math/mat3.h>
#include <etude/math/rect.h>
#include <etude/math/size.h>
#include <etude/math/vec2.h>

namespace etude {

    /// @brief Decides which part of the world the viewport shows, by default the whole window. The position lies in the
    /// top left corner of the viewport, and at zoom 1 one unit of the world covers one pixel.
    struct Camera2D {
        Vec2 position;
        float zoom = 1.0f;

        /// @brief Returns the matrix that maps world coordinates to clip space for a window of the given size, which
        /// the camera fills.
        constexpr Mat3 viewProjection(Size window) const {
            const Rect whole{
                .size = {static_cast<float>(window.width), static_cast<float>(window.height)},
            };
            return viewProjection(window, whole);
        }

        /// @brief Returns the matrix that maps world coordinates to clip space for a window of the given size, so that
        /// the camera fills the viewport, a rectangle of the window in pixels from its top left corner. Clip space
        /// still covers the whole window, so the world goes on beyond the viewport unless a clip rectangle cuts it off.
        constexpr Mat3 viewProjection(Size window, Rect viewport) const {
            const Rect view{
                .position = position - viewport.position * (1.0f / zoom),
                .size = {static_cast<float>(window.width) / zoom, static_cast<float>(window.height) / zoom},
            };
            return Mat3::orthographic(view);
        }
    };
}
