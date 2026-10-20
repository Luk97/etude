#pragma once

#include <etude/editor/command.h>
#include <etude/json/json.h>
#include <etude/scene/component_registry.h>
#include <etude/scene/entity.h>
#include <etude/scene/world.h>

#include <optional>

namespace etude::editor {

    /// @brief Turns what the UI does to one entity over several frames into one command, such as dragging a number of
    /// the inspector from the press to the release, or typing into a field from gaining to losing the focus.
    ///
    /// In every frame, watch comes before the UI may change the entity and settle after it. A gesture opens with the
    /// first change and closes in the first frame in which nothing holds it any more.
    class Gesture {
    public:
        /// @brief Notes the state of the entity before the UI of this frame can change it, unless a gesture is open,
        /// which keeps its entity and the state from its start.
        void watch(const World& world, const ComponentRegistry& registry, std::optional<Entity> entity);

        /// @brief Opens a gesture if the entity has changed, and closes an open one once ongoing is false, such as when
        /// the mouse button is up and no field has the focus. Returns the command of a closed gesture, unless the
        /// entity ended as it began. A gesture whose entity is gone ends without a command, since whatever destroyed
        /// the entity took over its state.
        std::optional<Command> settle(const World& world, const ComponentRegistry& registry, bool ongoing);

        /// @brief Returns whether a gesture is open, during which undo and redo would get mixed up with it.
        bool open() const;

    private:
        std::optional<Entity> watched;
        Json::Object before;
        bool changed = false;
    };
}
