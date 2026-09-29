#pragma once

#include <etude/rendering/renderer.h>
#include <etude/runtime/render_system.h>

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

namespace etude {

    /// @brief Loads each texture that scenes name once from the asset folder and keeps it on the GPU.
    class TextureCache {
    public:
        TextureCache(Renderer& renderer, std::filesystem::path assetFolder);

        /// @brief Returns the texture at the path relative to the asset folder and loads it the first time, which makes
        /// that frame wait for the copy to the GPU. A texture that cannot be loaded is logged once and gives nothing
        /// from then on, so that it does not flood the log in every frame.
        std::optional<LoadedTexture> get(const std::string& path);

    private:
        Renderer& renderer;
        std::filesystem::path assetFolder;
        std::unordered_map<std::string, std::optional<LoadedTexture>> textures;
    };
}
