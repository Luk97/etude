#pragma once

#include <etude/render2d/camera2d.h>
#include <etude/rendering/draw_batch.h>
#include <etude/rendering/sprite.h>

#include <vector>

namespace etude {

    /// @brief What a frame shows: the camera and the sprites in drawing order, so later sprites cover earlier ones.
    struct RenderList {
        Camera2D camera;
        std::vector<Sprite> sprites;

        /// @brief Batches that cover all sprites, each with its own matrix and clip rectangle, such as those of the UI.
        /// Their sprites have to stay alive until the frame is drawn.
        std::vector<DrawBatch> batches;
    };
}
