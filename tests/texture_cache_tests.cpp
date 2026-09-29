#include <catch2/catch_test_macros.hpp>

#include <etude/runtime/texture_cache.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <vector>

namespace {

    // A renderer without a GPU, which only counts the textures that it is given.
    class CountingRenderer : public etude::Renderer {
    public:
        void render(std::span<const etude::DrawBatch>) override {}

        void setClearColor(etude::Color) override {}

        etude::TextureId createTexture(const etude::Image&) override {
            return static_cast<etude::TextureId>(created++);
        }

        etude::FrameTimes frameTimes() const override {
            return {};
        }

        std::uint32_t created = 0;
    };

    // Two red pixels in the QOI format: the header, one RGB chunk, a run of one more pixel and the end marker.
    const std::vector<std::uint8_t> redPixels{
        'q', 'o', 'i', 'f', 0, 0, 0, 2, 0, 0, 0, 1, 4, 0, 0xfe, 255, 0, 0, 0xc0, 0, 0, 0, 0, 0, 0, 0, 1,
    };

    void writeImage(const std::filesystem::path& path) {
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(redPixels.data()), static_cast<std::streamsize>(redPixels.size()));
    }
}

TEST_CASE("TextureCache loads each texture once and knows the size of its image") {
    const std::filesystem::path folder = std::filesystem::temp_directory_path();
    writeImage(folder / "etude_texture_cache_test.qoi");

    CountingRenderer renderer;
    etude::TextureCache cache(renderer, folder);
    const auto first = cache.get("etude_texture_cache_test.qoi");
    const auto second = cache.get("etude_texture_cache_test.qoi");
    std::filesystem::remove(folder / "etude_texture_cache_test.qoi");

    REQUIRE(first);
    REQUIRE(second);
    CHECK(first->id == second->id);
    CHECK(first->size == etude::Vec2{2.0f, 1.0f});
    CHECK(renderer.created == 1);
}

TEST_CASE("TextureCache remembers a texture that it cannot load instead of trying again in every frame") {
    const std::filesystem::path folder = std::filesystem::temp_directory_path();
    CountingRenderer renderer;
    etude::TextureCache cache(renderer, folder);
    CHECK_FALSE(cache.get("etude_late_texture.qoi"));

    writeImage(folder / "etude_late_texture.qoi");
    const auto later = cache.get("etude_late_texture.qoi");
    std::filesystem::remove(folder / "etude_late_texture.qoi");
    CHECK_FALSE(later);
    CHECK(renderer.created == 0);
}
