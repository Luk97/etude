#include <etude/editor/history/edit_history.h>

#include <etude/editor/history/command.h>
#include <etude/editor/history/entity_change.h>
#include <etude/scene/components/name.h>
#include <etude/scene/components/transform2d.h>
#include <etude/scene/scene_json.h>

#include <utility>

namespace etude::editor {

    EditHistory::EditHistory(World& sceneWorld, const ComponentRegistry& componentTypes)
        : world(sceneWorld), registry(componentTypes) {}

    void EditHistory::watch(std::optional<Entity> selection) {
        gesture.watch(world, registry, selection);
    }

    void EditHistory::settle(bool ongoing) {
        if (std::optional<Command> command = gesture.settle(world, registry, ongoing)) {
            commands.push(std::move(*command));
        }
    }

    bool EditHistory::busy() const {
        return gesture.open();
    }

    Entity EditHistory::create(Vec2 position) {
        // The new entity gets a name for the hierarchy and a place in the scene, and nothing else.
        const Entity entity = world.create();
        const Transform2D transform{
            .position = position,
        };
        world.add(entity, Name{"Entity"});
        world.add(entity, transform);

        // It had no state before, so undo destroys it again.
        EntityChange change{
            .entity = entity,
            .before = std::nullopt,
            .after = writeEntity(world, entity, registry),
        };
        commands.push({
            .changes = {std::move(change)},
        });
        return entity;
    }

    void EditHistory::destroy(Entity entity) {
        // The state has to be written while the entity still exists, so that undo can bring it back.
        EntityChange change{
            .entity = entity,
            .before = writeEntity(world, entity, registry),
            .after = std::nullopt,
        };
        world.destroy(entity);
        commands.push({
            .changes = {std::move(change)},
        });
    }

    bool EditHistory::undo() {
        return commands.undo(world, registry);
    }

    bool EditHistory::redo() {
        return commands.redo(world, registry);
    }
}
