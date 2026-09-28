#pragma once

#include <etude/core/assert.h>
#include <etude/scene/entity.h>
#include <etude/scene/sparse_set.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace etude {

    /// @brief Holds the entities of a scene and their components, with one storage per component type.
    class World {
    public:
        /// @brief Creates an entity, reusing the index of a destroyed one with the next generation if there is one.
        Entity create();

        /// @brief Destroys the entity together with its components, after which it and all copies of its handle no
        /// longer count as alive. Destroying an entity that is not alive does nothing, so two systems may both destroy
        /// the same entity.
        void destroy(Entity entity);

        /// @brief Returns whether the entity was created by this world and has not been destroyed since.
        bool alive(Entity entity) const;

        /// @brief Returns how many entities are alive.
        std::size_t size() const;

        /// @brief Adds the component to the entity, or replaces the one it has, and returns the stored component. The
        /// reference stays valid until a component of the same type is added or removed, also through destroy.
        template <typename Component>
        Component& add(Entity entity, Component component) {
            ETUDE_ASSERT(alive(entity));
            return storage<Component>().add(entity, std::move(component));
        }

        /// @brief Removes the component from the entity, if it has one.
        template <typename Component>
        void remove(Entity entity) {
            if (SparseSet<Component>* components = findStorage<Component>()) {
                components->remove(entity);
            }
        }

        /// @brief Returns whether the entity has a component of this type.
        template <typename Component>
        bool has(Entity entity) const {
            return tryGet<Component>(entity) != nullptr;
        }

        /// @brief Returns the component of the entity, or nullptr if it has none.
        template <typename Component>
        auto* tryGet(this auto& self, Entity entity) {
            auto* components = self.template findStorage<Component>();
            return components != nullptr ? components->find(entity) : nullptr;
        }

        /// @brief Returns the component of the entity, which has to have one. Deducing this writes the const and the
        /// non-const version at once: a const world hands out const components.
        template <typename Component>
        auto& get(this auto& self, Entity entity) {
            auto* component = self.template tryGet<Component>(entity);
            ETUDE_ASSERT(component != nullptr);
            return *component;
        }

        /// @brief Calls the callback with each entity that has all of the components, together with references to
        /// them. It runs through the smallest of their storages from the back, so the callback may destroy the current
        /// entity or remove its components: only entities that were visited already move into its place. Destroying
        /// other entities during the loop is not safe.
        template <typename... Components>
            requires(sizeof...(Components) > 0)
        void each(this auto& self, auto&& callback) {
            eachIn(callback, self.template findStorage<Components>()...);
        }

    private:
        /// @brief Gives T the constness of the world that Self refers to.
        template <typename Self, typename T>
        using ConstLike = std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const T, T>;

        /// @brief Numbers the component types in the order in which the program first uses them, so that each type
        /// finds its storage. The numbers can change between runs, which is why scene files will name the types.
        template <typename Component>
        static std::size_t typeIndex() {
            static const std::size_t index = nextTypeIndex();
            return index;
        }

        static std::size_t nextTypeIndex();

        /// @brief Returns the storage of the component type, or nullptr if this world has none yet. The storages sit
        /// behind pointers, which do not pass the constness of the world on by themselves.
        template <typename Component>
        auto* findStorage(this auto& self) {
            using Storage = ConstLike<decltype(self), SparseSet<Component>>;
            const std::size_t index = typeIndex<Component>();
            return index < self.storages.size() ? static_cast<Storage*>(self.storages[index].get()) : nullptr;
        }

        /// @brief Returns the storage of the component type and creates it the first time.
        template <typename Component>
        SparseSet<Component>& storage() {
            const std::size_t index = typeIndex<Component>();
            if (index >= storages.size()) {
                storages.resize(index + 1);
            }
            if (!storages[index]) {
                storages[index] = std::make_unique<SparseSet<Component>>();
            }
            return static_cast<SparseSet<Component>&>(*storages[index]);
        }

        /// @brief Does the work of each once the storages are looked up. Without a storage for one of the component
        /// types, no entity can have all of them.
        static void eachIn(auto& callback, auto*... sets) {
            if ((... || (sets == nullptr))) {
                return;
            }
            const auto bySize = [](const auto* left, const auto* right) { return left->size() < right->size(); };
            const std::vector<Entity>* entities = std::min({&sets->entities()...}, bySize);

            // A span would dangle if the callback adds a component and the vector reallocates, so index the vector.
            for (std::size_t i = entities->size(); i > 0; --i) {
                const Entity entity = (*entities)[i - 1];
                if ((... && sets->contains(entity))) {
                    callback(entity, *sets->find(entity)...);
                }
            }
        }

        /// @brief The current generation of each index. Destroying an entity counts it up, which kills its old handles.
        std::vector<std::uint32_t> generations;

        /// @brief Indices of destroyed entities, which create hands out again, the most recently freed first.
        std::vector<std::uint32_t> freeIndices;

        /// @brief One storage per component type, at the number of the type. A type that this world has not used yet
        /// has none.
        std::vector<std::unique_ptr<ComponentStorage>> storages;
    };
}
