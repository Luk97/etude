#include <etude/scene/scene_file.h>

#include <etude/core/file.h>
#include <etude/json/json.h>
#include <etude/scene/scene_json.h>

#include <format>
#include <string_view>

namespace etude {

    std::expected<World, std::string> loadScene(const std::filesystem::path& path, const ComponentRegistry& registry) {
        const auto bytes = readFile(path);
        if (!bytes) {
            return std::unexpected(std::format("Cannot load {}: {}.", path.string(), bytes.error()));
        }

        // JSON text is UTF-8, whose bytes a std::string_view holds as chars.
        const std::string_view text(reinterpret_cast<const char*>(bytes->data()), bytes->size());
        const auto json = parseJson(text);
        if (!json) {
            const JsonError& error = json.error();
            return std::unexpected(
                std::format("Cannot load {}:{}:{}: {}.", path.string(), error.line, error.column, error.message)
            );
        }
        return readScene(*json, registry).transform_error([&](const std::string& error) {
            return std::format("Cannot load {}: {}.", path.string(), error);
        });
    }

    std::expected<void, std::string> saveScene(
        const std::filesystem::path& path,
        const World& world,
        const ComponentRegistry& registry
    ) {
        const Json json = writeScene(world, registry);
        // JSON text has no NaN and no infinity, so a scene that holds one would save but never load again.
        if (const auto check = readScene(json, registry); !check) {
            return std::unexpected(
                std::format("Cannot save {}, it would not load again: {}.", path.string(), check.error())
            );
        }
        const std::string text = writeJson(json) + "\n";
        return writeFile(path, text).transform_error([&](const std::string& error) {
            return std::format("Cannot save {}: {}.", path.string(), error);
        });
    }
}
