#pragma once

#include <etude/scene/component_registry.h>
#include <etude/scene/world.h>

#include <expected>
#include <filesystem>
#include <string>

namespace etude {

    /// @brief Reads the scene file into a new world. Returns why instead, with the path and, for broken JSON, with
    /// line and column, so that the message points at the mistake.
    std::expected<World, std::string> loadScene(const std::filesystem::path& path, const ComponentRegistry& registry);

    /// @brief Writes the world into the scene file as writeJson formats it, with a line break at the end like any
    /// text file. Returns why instead if the file cannot be written.
    std::expected<void, std::string> saveScene(
        const std::filesystem::path& path,
        const World& world,
        const ComponentRegistry& registry
    );
}
