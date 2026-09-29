#include <etude/scene/world.h>

#include <etude/core/assert.h>

namespace etude {

    Entity World::create() {
        if (!freeIndices.empty()) {
            const std::uint32_t index = freeIndices.back();
            freeIndices.pop_back();
            occupied[index] = true;
            return makeEntity(index, generations[index]);
        }

        ETUDE_CHECK(generations.size() <= entityIndexMask);
        generations.push_back(0);
        occupied.push_back(true);
        return makeEntity(static_cast<std::uint32_t>(generations.size() - 1), 0);
    }

    void World::destroy(Entity entity) {
        if (!alive(entity)) {
            return;
        }
        for (const std::unique_ptr<ComponentStorage>& components : storages) {
            if (components) {
                components->remove(entity);
            }
        }

        const std::uint32_t index = indexOf(entity);
        generations[index] = (generations[index] + 1) & entityGenerationMask;
        occupied[index] = false;
        freeIndices.push_back(index);
    }

    bool World::alive(Entity entity) const {
        const std::uint32_t index = indexOf(entity);
        return index < generations.size() && occupied[index] && generations[index] == generationOf(entity);
    }

    std::size_t World::size() const {
        return generations.size() - freeIndices.size();
    }

    std::vector<Entity> World::entities() const {
        std::vector<Entity> result;
        result.reserve(size());
        for (std::uint32_t index = 0; index < generations.size(); ++index) {
            if (occupied[index]) {
                result.push_back(makeEntity(index, generations[index]));
            }
        }
        return result;
    }

    std::size_t World::nextTypeIndex() {
        static std::size_t next = 0;
        return next++;
    }
}
