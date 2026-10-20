#include <etude/editor/entity_commands.h>

#include <etude/scene/components/name.h>
#include <etude/scene/components/transform2d.h>
#include <etude/scene/scene_json.h>

#include <optional>
#include <utility>

namespace etude::editor {

    Entity createEntity(World& world, const ComponentRegistry& registry, History& history, Vec2 position) {
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
        history.push({
            .changes = {std::move(change)},
        });
        return entity;
    }

    void destroyEntity(World& world, const ComponentRegistry& registry, History& history, Entity entity) {
        // The state has to be written while the entity still exists, so that undo can bring it back.
        EntityChange change{
            .entity = entity,
            .before = writeEntity(world, entity, registry),
            .after = std::nullopt,
        };
        world.destroy(entity);
        history.push({
            .changes = {std::move(change)},
        });
    }
}
