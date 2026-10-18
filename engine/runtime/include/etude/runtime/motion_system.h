#pragma once

#include <etude/scene/world.h>

#include <chrono>

namespace etude {

    /// @brief Moves each entity with a Transform2D and a Velocity on by its velocity over the given time, as one step
    /// of the simulation does.
    void moveScene(World& world, std::chrono::duration<float> time);
}
