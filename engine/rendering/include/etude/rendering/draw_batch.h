#pragma once

#include <etude/math/mat3.h>
#include <etude/math/rect.h>
#include <etude/rendering/sprite.h>

#include <optional>
#include <span>

namespace etude {

    /// @brief Sprites that share a matrix and a clip rectangle, such as those of the world or of one UI panel.
    struct DrawBatch {

        /// @brief Maps the coordinates of the sprites to clip space, for the world through its camera.
        Mat3 viewProjection;

        /// @brief The part of the window in pixels that the sprites may cover, measured from its top left corner. The
        /// whole window if left out.
        std::optional<Rect> clip;

        std::span<const Sprite> sprites;
    };
}
