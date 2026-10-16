#pragma once

#include <etude/json/json.h>
#include <etude/scene/component_registry.h>
#include <etude/scene/entity.h>
#include <etude/scene/world.h>

#include <expected>
#include <string>

namespace etude {

    /// @brief The version of the scene format. It goes up whenever the format changes in a way that older readers
    /// would misunderstand.
    inline constexpr int sceneVersion = 1;

    /// @brief Writes the components of the entity as JSON, in the order of the registry and keyed by the names of their
    /// types, as one entity of a scene file. Components whose type the registry does not know stay out.
    Json::Object writeEntity(const World& world, Entity entity, const ComponentRegistry& registry);

    /// @brief Gives the entity exactly the components of the registry that the JSON holds, as writeEntity writes them:
    /// it reads those and removes the others. Components whose type the registry does not know stay as they are.
    /// Returns the first error instead, with its place in the entity such as Camera.zoom, after which the entity may
    /// keep what was read so far.
    std::expected<void, std::string> readEntity(
        World& world,
        Entity entity,
        const Json::Object& components,
        const ComponentRegistry& registry
    );

    /// @brief Writes the entities of the world in the order of their indices, each with its components in the order of
    /// the registry. Components whose type the registry does not know stay out of the scene.
    Json writeScene(const World& world, const ComponentRegistry& registry);

    /// @brief Reads a scene as writeScene writes it into a new world, whose entities follow the order of the file.
    /// Returns the first error with its place in the scene instead, such as entities[2].Transform2D.rotation.
    std::expected<World, std::string> readScene(const Json& json, const ComponentRegistry& registry);
}
