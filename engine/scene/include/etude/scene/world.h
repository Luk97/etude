#pragma once

#include <etude/scene/entity.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace etude {

    /// @brief Holds the entities of a scene.
    class World {
    public:
        /// @brief Creates an entity, reusing the index of a destroyed one with the next generation if there is one.
        Entity create();

        /// @brief Destroys the entity, after which it and all copies of its handle no longer count as alive. Destroying
        /// an entity that is not alive does nothing, so two systems may both destroy the same entity.
        void destroy(Entity entity);

        /// @brief Returns whether the entity was created by this world and has not been destroyed since.
        bool alive(Entity entity) const;

        /// @brief Returns how many entities are alive.
        std::size_t size() const;

    private:
        /// @brief The current generation of each index. Destroying an entity counts it up, which kills its old handles.
        std::vector<std::uint32_t> generations;

        /// @brief Indices of destroyed entities, which create hands out again, the most recently freed first.
        std::vector<std::uint32_t> freeIndices;
    };
}
