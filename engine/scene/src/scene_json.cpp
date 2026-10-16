#include <etude/scene/scene_json.h>

#include <algorithm>
#include <cstddef>
#include <format>
#include <optional>
#include <utility>

namespace etude {

    Json::Object writeEntity(const World& world, Entity entity, const ComponentRegistry& registry) {
        Json::Object components;
        for (const ComponentType& type : registry.types()) {
            if (std::optional<Json> component = type.write(world, entity)) {
                components.emplace_back(type.name, std::move(*component));
            }
        }
        return components;
    }

    std::expected<void, std::string> readEntity(
        World& world,
        Entity entity,
        const Json::Object& components,
        const ComponentRegistry& registry
    ) {
        // Checked first, so that a typing mistake in a name is found before anything changes.
        for (const auto& member : components) {
            if (registry.find(member.first) == nullptr) {
                return std::unexpected(std::format("{} is an unknown component", member.first));
            }
        }

        // Each type of the registry reads its component from the JSON, or loses the one it has if the JSON has none.
        for (const ComponentType& type : registry.types()) {
            const auto member = std::ranges::find(components, type.name, &Json::Object::value_type::first);
            if (member == components.end()) {
                type.remove(world, entity);
            } else if (const auto read = type.read(world, entity, member->second); !read) {
                return std::unexpected(read.error());
            }
        }
        return {};
    }

    Json writeScene(const World& world, const ComponentRegistry& registry) {
        Json::Array entities;
        for (const Entity entity : world.entities()) {
            entities.emplace_back(writeEntity(world, entity, registry));
        }

        Json::Object scene;
        scene.emplace_back("version", sceneVersion);
        scene.emplace_back("entities", std::move(entities));
        return scene;
    }

    std::expected<World, std::string> readScene(const Json& json, const ComponentRegistry& registry) {
        const Json* version = json.find("version");
        if (version == nullptr || *version != Json(sceneVersion)) {
            return std::unexpected(std::format("the scene has to have version {}", sceneVersion));
        }

        const Json* member = json.find("entities");
        const Json::Array* entities = member != nullptr ? member->tryGet<Json::Array>() : nullptr;
        if (entities == nullptr) {
            return std::unexpected("the scene has to have an array of entities");
        }

        World world;
        for (std::size_t i = 0; i < entities->size(); ++i) {
            const Json::Object* components = (*entities)[i].tryGet<Json::Object>();
            if (components == nullptr) {
                return std::unexpected(std::format("entities[{}] has to be an object", i));
            }
            if (const auto read = readEntity(world, world.create(), *components, registry); !read) {
                return std::unexpected(std::format("entities[{}].{}", i, read.error()));
            }
        }
        return world;
    }
}
