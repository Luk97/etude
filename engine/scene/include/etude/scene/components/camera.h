#pragma once

#include <etude/scene/reflection.h>

#include <string_view>
#include <tuple>

namespace etude {

    /// @brief Shows the world around its entity, whose position lies in the middle of the picture. At zoom 1, one unit
    /// of the world covers one pixel.
    struct Camera {
        float zoom = 1.0f;

        static constexpr std::string_view typeName = "Camera";
        static constexpr std::tuple fields{
            Field{"zoom", &Camera::zoom},
        };
    };
}
