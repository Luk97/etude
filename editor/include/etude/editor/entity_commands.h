#pragma once

#include <etude/editor/history.h>
#include <etude/math/vec2.h>
#include <etude/scene/component_registry.h>
#include <etude/scene/entity.h>
#include <etude/scene/world.h>

namespace etude::editor {

    /// @brief Creates an entity named Entity with a Transform2D at the position and records it in the history, so that
    /// undo takes it away again. Returns the new entity.
    Entity createEntity(World& world, const ComponentRegistry& registry, History& history, Vec2 position);

    /// @brief Destroys the entity and records it in the history, so that undo brings it back under its old handle with
    /// all its components.
    void destroyEntity(World& world, const ComponentRegistry& registry, History& history, Entity entity);
}
