#pragma once

#include <etude/render2d/camera2d.h>
#include <etude/rendering/sprite.h>

#include <vector>

namespace etude {

    /// @brief What a frame shows: the camera and the sprites in drawing order, so later sprites cover earlier ones.
    struct RenderList {
        Camera2D camera;
        std::vector<Sprite> sprites;
    };
}
