#pragma once

#include <etude/math/vec2.h>
#include <etude/scene/reflection.h>

#include <string_view>
#include <tuple>

namespace etude {

    /// @brief Moves the entity on its own while the simulation runs: linear in units of the world per second, angular
    /// in degrees per second, clockwise like the rotation of Transform2D.
    struct Velocity {
        Vec2 linear;
        float angular = 0.0f;

        static constexpr std::string_view typeName = "Velocity";
        static constexpr std::tuple fields{
            Field{"linear", &Velocity::linear},
            Field{"angular", &Velocity::angular},
        };
    };
}
