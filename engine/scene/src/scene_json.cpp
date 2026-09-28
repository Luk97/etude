#include <etude/scene/scene_json.h>

#include <cstddef>
#include <format>
#include <optional>
#include <utility>

namespace etude {

    Json writeScene(const World& world, const ComponentRegistry& registry) {
        Json::Array entities;
        for (const Entity entity : world.entities()) {
            Json::Object components;
            for (const ComponentType& type : registry.types()) {
                if (std::optional<Json> component = type.write(world, entity)) {
                    components.emplace_back(type.name, std::move(*component));
                }
            }
            entities.emplace_back(std::move(components));
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
            const Entity entity = world.create();
            for (const auto& [name, component] : *components) {
                const ComponentType* type = registry.find(name);
                if (type == nullptr) {
                    return std::unexpected(std::format("entities[{}] has an unknown component {}", i, name));
                }
                if (const auto read = type->read(world, entity, component); !read) {
                    return std::unexpected(std::format("entities[{}].{}", i, read.error()));
                }
            }
        }
        return world;
    }
}
