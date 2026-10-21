#include <etude/editor/history/command_stack.h>

#include <etude/core/assert.h>
#include <etude/scene/scene_json.h>

#include <optional>
#include <ranges>
#include <utility>

namespace etude::editor {

    namespace {

        /// @brief Gives the entity the state: none destroys it, otherwise it comes back under its handle if it is gone
        /// and gets exactly the components of the state.
        void restore(
            World& world,
            const ComponentRegistry& registry,
            Entity entity,
            const std::optional<Json::Object>& state
        ) {
            if (!state) {
                world.destroy(entity);
                return;
            }
            if (!world.alive(entity)) {
                world.recreate(entity);
            }

            // The state came from writeEntity, so reading it back cannot fail.
            [[maybe_unused]] const auto read = readEntity(world, entity, *state, registry);
            ETUDE_ASSERT(read);
        }
    }

    void CommandStack::push(Command command) {
        commands.resize(done);
        commands.push_back(std::move(command));
        ++done;
    }

    bool CommandStack::undo(World& world, const ComponentRegistry& registry) {
        if (done == 0) {
            return false;
        }
        --done;
        for (const EntityChange& change : commands[done].changes | std::views::reverse) {
            restore(world, registry, change.entity, change.before);
        }
        return true;
    }

    bool CommandStack::redo(World& world, const ComponentRegistry& registry) {
        if (done == commands.size()) {
            return false;
        }
        for (const EntityChange& change : commands[done].changes) {
            restore(world, registry, change.entity, change.after);
        }
        ++done;
        return true;
    }
}
