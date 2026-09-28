#include <etude/scene/world.h>

#include <etude/core/assert.h>

namespace etude {

    Entity World::create() {
        if (!freeIndices.empty()) {
            const std::uint32_t index = freeIndices.back();
            freeIndices.pop_back();
            return makeEntity(index, generations[index]);
        }

        ETUDE_ASSERT(generations.size() <= entityIndexMask);
        generations.push_back(0);
        return makeEntity(static_cast<std::uint32_t>(generations.size() - 1), 0);
    }

    void World::destroy(Entity entity) {
        if (!alive(entity)) {
            return;
        }
        const std::uint32_t index = indexOf(entity);
        generations[index] = (generations[index] + 1) & entityGenerationMask;
        freeIndices.push_back(index);
    }

    bool World::alive(Entity entity) const {
        const std::uint32_t index = indexOf(entity);
        return index < generations.size() && generations[index] == generationOf(entity);
    }

    std::size_t World::size() const {
        return generations.size() - freeIndices.size();
    }
}
