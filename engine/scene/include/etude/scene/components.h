#pragma once

#include <etude/math/vec2.h>
#include <etude/scene/reflection.h>

#include <string>
#include <string_view>
#include <tuple>

namespace etude {

    /// @brief Names an entity for the editor and for scripts. Several entities may have the same name.
    struct Name {
        std::string value;

        static constexpr std::string_view typeName = "Name";
        static constexpr std::tuple fields{
            Field{"value", &Name::value},
        };
    };

    /// @brief Places an entity in the world, where y points down. The position is the middle of the entity, the
    /// rotation in degrees turns it clockwise on the screen around that point, and the scale stretches it.
    struct Transform2D {
        Vec2 position;
        float rotation = 0.0f;
        Vec2 scale{1.0f, 1.0f};

        static constexpr std::string_view typeName = "Transform2D";
        static constexpr std::tuple fields{
            Field{"position", &Transform2D::position},
            Field{"rotation", &Transform2D::rotation},
            Field{"scale", &Transform2D::scale},
        };
    };

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
