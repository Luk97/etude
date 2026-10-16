#pragma once

#include <etude/json/json.h>
#include <etude/scene/entity.h>
#include <etude/scene/field_visitor.h>
#include <etude/scene/world.h>

#include <expected>
#include <optional>
#include <string>
#include <string_view>

namespace etude {

    /// @brief What the registry knows about a component type: its fixed name and how to work with the component of an
    /// entity without knowing the type, as scene files and the editor need it.
    struct ComponentType {
        std::string_view name;

        /// @brief Returns whether the entity has a component of this type.
        bool (*has)(const World& world, Entity entity) = nullptr;

        /// @brief Returns the component of the entity as JSON, or nothing if the entity has none.
        std::optional<Json> (*write)(const World& world, Entity entity) = nullptr;

        /// @brief Reads the component from JSON and adds it to the entity, replacing one that it already has.
        std::expected<void, std::string> (*read)(World& world, Entity entity, const Json& json) = nullptr;

        /// @brief Removes the component from the entity, if it has one.
        void (*remove)(World& world, Entity entity) = nullptr;

        /// @brief Hands each field of the component to the visitor, which may change it. The entity has to have a
        /// component of this type.
        void (*inspect)(World& world, Entity entity, FieldVisitor& visitor) = nullptr;
    };
}
