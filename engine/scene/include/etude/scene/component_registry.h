#pragma once

#include <etude/core/assert.h>
#include <etude/json/json.h>
#include <etude/scene/component_json.h>
#include <etude/scene/entity.h>
#include <etude/scene/reflection.h>
#include <etude/scene/world.h>

#include <expected>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace etude {

    /// @brief What the registry knows about a component type: its fixed name and how to read and write the component
    /// of an entity as JSON.
    struct ComponentType {
        std::string_view name;

        /// @brief Returns the component of the entity as JSON, or nothing if the entity has none.
        std::optional<Json> (*write)(const World& world, Entity entity) = nullptr;

        /// @brief Reads the component from JSON and adds it to the entity, replacing one that it already has.
        std::expected<void, std::string> (*read)(World& world, Entity entity, const Json& json) = nullptr;
    };

    /// @brief Knows the component types that scenes can hold by their fixed names, so that files do not depend on the
    /// numbers that the world gives the types.
    class ComponentRegistry {
    public:
        /// @brief Adds the component type, whose name must not be taken yet. Scenes list the components of an entity
        /// in the order in which their types were added.
        template <Reflected Component>
        void add() {
            ETUDE_ASSERT(find(Component::typeName) == nullptr);
            componentTypes.push_back({
                .name = Component::typeName,
                .write = &write<Component>,
                .read = &read<Component>,
            });
        }

        /// @brief Returns the type with the name, or nullptr if there is none. The pointer stays valid until the next
        /// add.
        const ComponentType* find(std::string_view name) const;

        /// @brief Returns the types in the order in which they were added.
        std::span<const ComponentType> types() const {
            return componentTypes;
        }

    private:
        template <Reflected Component>
        static std::optional<Json> write(const World& world, Entity entity) {
            const Component* component = world.tryGet<Component>(entity);
            if (component == nullptr) {
                return std::nullopt;
            }
            return writeComponent(*component);
        }

        template <Reflected Component>
        static std::expected<void, std::string> read(World& world, Entity entity, const Json& json) {
            return readComponent<Component>(json).transform([&](Component component) {
                world.add(entity, std::move(component));
            });
        }

        std::vector<ComponentType> componentTypes;
    };

    /// @brief Adds the components that come with the engine: Name, Transform2D, SpriteRenderer and Camera.
    void addBuiltinComponents(ComponentRegistry& registry);
}
