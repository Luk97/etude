#pragma once

#include <etude/editor/history/command_stack.h>
#include <etude/editor/history/gesture.h>
#include <etude/math/vec2.h>
#include <etude/scene/component_registry.h>
#include <etude/scene/entity.h>
#include <etude/scene/world.h>

#include <optional>

namespace etude::editor {

    /// @brief Everything that the editor changes in the world, so that undo can take it back and redo can do it again.
    /// What the UI does to the selection over several frames becomes one command between watch and settle, such as a
    /// whole drag, and creating and destroying an entity become commands of their own. The world and the component
    /// types have to outlive the history.
    class EditHistory {
    public:
        EditHistory(World& sceneWorld, const ComponentRegistry& componentTypes);

        /// @brief Notes the selection before the UI of this frame may change it, unless an edit of several frames is
        /// open, which keeps the entity and the state from its start.
        void watch(std::optional<Entity> selection);

        /// @brief After the UI of this frame, closes an open edit once ongoing is false, such as when the mouse button
        /// is up and no field has the focus, and keeps it as a command unless the entity ended as it began.
        void settle(bool ongoing);

        /// @brief Returns whether an edit of several frames is open, during which undo and redo would get mixed up with
        /// it.
        bool busy() const;

        /// @brief Creates an entity named Entity with a Transform2D at the position and returns it.
        Entity create(Vec2 position);

        /// @brief Destroys the entity, which undo brings back under its old handle with all its components.
        void destroy(Entity entity);

        /// @brief Takes back the last command. Returns false if there is nothing to take back.
        bool undo();

        /// @brief Does the last command that undo took back again. Returns false if there is nothing to do again.
        bool redo();

    private:
        World& world;
        const ComponentRegistry& registry;
        CommandStack commands;
        Gesture gesture;
    };
}
