#include <etude/editor/history/gesture.h>

#include <etude/scene/scene_json.h>

#include <utility>

namespace etude::editor {

    void Gesture::watch(const World& world, const ComponentRegistry& registry, std::optional<Entity> entity) {
        if (changed) {
            return;
        }
        watched = entity;
        if (entity) {
            before = writeEntity(world, *entity, registry);
        }
    }

    std::optional<Command> Gesture::settle(const World& world, const ComponentRegistry& registry, bool ongoing) {
        // Without an entity there is nothing to settle, and a gesture whose entity is gone ends without a command.
        if (!watched) {
            return std::nullopt;
        }
        if (!world.alive(*watched)) {
            changed = false;
            watched.reset();
            return std::nullopt;
        }

        // Once the entity differs from its start, the gesture stays open until nothing holds it, even if the entity
        // changes back on the way.
        Json::Object after = writeEntity(world, *watched, registry);
        changed = changed || after != before;
        if (!changed || ongoing) {
            return std::nullopt;
        }

        // The gesture closes, and it only gives a command if the entity ended up different from its start.
        changed = false;
        if (after == before) {
            return std::nullopt;
        }
        EntityChange change{
            .entity = *watched,
            .before = std::move(before),
            .after = std::move(after),
        };
        return Command{
            .changes = {std::move(change)},
        };
    }

    bool Gesture::open() const {
        return changed;
    }
}
