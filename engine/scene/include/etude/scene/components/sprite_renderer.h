#pragma once

#include <etude/scene/reflection.h>

#include <string>
#include <string_view>
#include <tuple>

namespace etude {

    /// @brief Draws the entity with a texture, whose path is relative to the asset folder and uses forward slashes.
    /// Sprites on higher layers cover those on lower ones.
    struct SpriteRenderer {
        std::string texture;
        int layer = 0;

        static constexpr std::string_view typeName = "SpriteRenderer";
        static constexpr std::tuple fields{
            Field{"texture", &SpriteRenderer::texture},
            Field{"layer", &SpriteRenderer::layer},
        };
    };
}
