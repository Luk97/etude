#include <etude/runtime/texture_cache.h>

#include <etude/assets/qoi.h>
#include <etude/core/log.h>

#include <utility>

namespace etude {

    TextureCache::TextureCache(Renderer& renderer, std::filesystem::path assetFolder)
        : renderer(renderer), assetFolder(std::move(assetFolder)) {}

    std::optional<LoadedTexture> TextureCache::get(const std::string& path) {
        if (const auto found = textures.find(path); found != textures.end()) {
            return found->second;
        }

        std::optional<LoadedTexture> texture;
        if (const auto image = loadQoi(assetFolder / path)) {
            texture = LoadedTexture{
                .id = renderer.createTexture(*image),
                .size = {static_cast<float>(image->width), static_cast<float>(image->height)},
            };
        } else {
            logError("{}", image.error());
        }
        textures.emplace(path, texture);
        return texture;
    }
}
