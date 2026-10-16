#pragma once

#include <etude/scene/entity.h>

#include <memory>

namespace etude {

    /// @brief What the world needs from the storage of a component type without knowing the type: removing the
    /// component of an entity that it destroys, and copying the whole storage when the world is copied.
    class ComponentStorage {
    public:
        virtual ~ComponentStorage() = default;

        /// @brief Removes the component of the entity, if it has one.
        virtual void remove(Entity entity) = 0;

        /// @brief Returns a copy of the storage with copies of all its components.
        virtual std::unique_ptr<ComponentStorage> clone() const = 0;
    };
}
