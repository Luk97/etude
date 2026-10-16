#pragma once

#include <etude/math/vec2.h>
#include <etude/scene/reflection.h>

#include <string_view>
#include <tuple>

namespace etude {

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
}
