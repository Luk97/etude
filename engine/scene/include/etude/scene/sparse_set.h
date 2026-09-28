#pragma once

#include <etude/core/assert.h>
#include <etude/scene/entity.h>

#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace etude {

    /// @brief What the world needs from the storage of a component type without knowing the type: removing the
    /// component of an entity that it destroys.
    class ComponentStorage {
    public:
        virtual ~ComponentStorage() = default;

        /// @brief Removes the component of the entity, if it has one.
        virtual void remove(Entity entity) = 0;
    };

    /// @brief Stores the components of one type without gaps, so that systems run through them in one go. A sparse
    /// array maps the index of an entity to the place of its component. Removing a component moves the last one into
    /// the gap, which keeps them packed but changes their order.
    template <typename Component>
    class SparseSet final : public ComponentStorage {
    public:
        /// @brief Adds the component to the entity, or replaces the one it has, and returns the stored component. No
        /// older entity with the same index may still have a component here, which the world ensures in destroy.
        Component& add(Entity entity, Component component) {
            if (Component* existing = find(entity)) {
                *existing = std::move(component);
                return *existing;
            }

            const std::uint32_t index = indexOf(entity);
            if (index >= sparse.size()) {
                sparse.resize(index + 1, absent);
            }
            ETUDE_ASSERT(sparse[index] == absent);
            sparse[index] = static_cast<std::uint32_t>(dense.size());
            dense.push_back(entity);
            values.push_back(std::move(component));
            return values.back();
        }

        void remove(Entity entity) override {
            if (!contains(entity)) {
                return;
            }
            const std::uint32_t place = sparse[indexOf(entity)];
            const std::uint32_t last = static_cast<std::uint32_t>(dense.size() - 1);
            if (place != last) {
                dense[place] = dense[last];
                values[place] = std::move(values[last]);
                sparse[indexOf(dense[place])] = place;
            }
            dense.pop_back();
            values.pop_back();
            sparse[indexOf(entity)] = absent;
        }

        /// @brief Returns whether the entity has a component here. The whole handle has to match, so an old handle
        /// does not see the component of a newer entity with the same index.
        bool contains(Entity entity) const {
            const std::uint32_t index = indexOf(entity);
            return index < sparse.size() && sparse[index] != absent && dense[sparse[index]] == entity;
        }

        /// @brief Returns the component of the entity, or nullptr if it has none. Deducing this writes the const and
        /// the non-const version at once: a const storage hands out const components.
        auto* find(this auto& self, Entity entity) {
            return self.contains(entity) ? &self.values[self.sparse[indexOf(entity)]] : nullptr;
        }

        /// @brief Returns the entities with a component here, in the same order as their components.
        const std::vector<Entity>& entities() const {
            return dense;
        }

    private:
        static constexpr std::uint32_t absent = std::numeric_limits<std::uint32_t>::max();

        /// @brief The place of the component for each entity index, or absent.
        std::vector<std::uint32_t> sparse;

        /// @brief The entities with a component, at the same places as their components.
        std::vector<Entity> dense;

        std::vector<Component> values;
    };
}
