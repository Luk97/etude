#include <etude/scene/world.h>

#include <etude/core/assert.h>

namespace etude {

    World::World(const World& other)
        : generations(other.generations), occupied(other.occupied), freeIndices(other.freeIndices) {
        // Only a storage knows the type of its components, so each one copies itself.
        storages.reserve(other.storages.size());
        for (const std::unique_ptr<ComponentStorage>& components : other.storages) {
            storages.push_back(components ? components->clone() : nullptr);
        }
    }

    World& World::operator=(const World& other) {
        // Copies first and moves the copy in, so that the copy constructor does the work and a world assigned to
        // itself stays whole.
        return *this = World(other);
    }

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

    void World::recreate(Entity entity) {
        const std::uint32_t index = indexOf(entity);
        ETUDE_ASSERT(index < occupied.size() && !occupied[index]);

        // The index can sit anywhere in the free ones, since others may have been freed after it.
        std::erase(freeIndices, index);
        generations[index] = generationOf(entity);
        occupied[index] = true;
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
