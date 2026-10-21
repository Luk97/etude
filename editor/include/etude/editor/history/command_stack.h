#pragma once

#include <etude/editor/history/command.h>
#include <etude/scene/component_registry.h>
#include <etude/scene/world.h>

#include <cstddef>
#include <vector>

namespace etude::editor {

    /// @brief The undo stack: the commands that are done and those that undo took back, which redo can do again. A new
    /// command throws away the ones that could be done again, since they started from a state that is gone.
    class CommandStack {
    public:
        /// @brief Adds a command that the editor has done already.
        void push(Command command);

        /// @brief Takes back the last command that is done: the entities that it touched get their states from before
        /// it, in the reverse order of its changes. Returns false if there is nothing to take back.
        bool undo(World& world, const ComponentRegistry& registry);

        /// @brief Does the last command that undo took back again: the entities get their states from after it. Returns
        /// false if there is nothing to do again.
        bool redo(World& world, const ComponentRegistry& registry);

    private:
        std::vector<Command> commands;

        /// @brief How many of the commands are done, the first ones. The rest can be done again.
        std::size_t done = 0;
    };
}
