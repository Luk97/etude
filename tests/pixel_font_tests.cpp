#include <catch2/catch_test_macros.hpp>

#include <etude/render2d/pixel_font.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

using etude::PixelFont;
using etude::Sprite;
using etude::Vec2;

namespace {

    constexpr auto fontTexture = static_cast<etude::TextureId>(7);

    constexpr etude::Color orange{
        .r = 1.0f,
        .g = 0.5f,
        .b = 0.0f,
    };

    // Reads the pixels that the sprite shows from the atlas, row by row with # for every set pixel. Starts with a line
    // break, like a raw string literal that begins on its own line.
    std::string shownPixels(const etude::Image& atlas, const Sprite& sprite) {
        const auto left = static_cast<int>(sprite.uv.position.x * atlas.width);
        const auto top = static_cast<int>(sprite.uv.position.y * atlas.height);
        const auto width = static_cast<int>(sprite.uv.size.x * atlas.width);
        const auto height = static_cast<int>(sprite.uv.size.y * atlas.height);
        std::string shown = "\n";
        for (int y = top; y < top + height; ++y) {
            for (int x = left; x < left + width; ++x) {
                shown += atlas.pixels[static_cast<std::size_t>(y * atlas.width + x)].a == 255 ? '#' : '.';
            }
            shown += '\n';
        }
        return shown;
    }

    std::vector<Sprite> textSprites(std::string_view text) {
        std::vector<Sprite> sprites;
        PixelFont(fontTexture).appendText(sprites, text, {}, 1, orange);
        return sprites;
    }
}

TEST_CASE("PixelFont shows each character as its glyph from the atlas, descenders included") {
    const std::vector<Sprite> sprites = textSprites("g");
    REQUIRE(sprites.size() == 1);
    CHECK(sprites[0].texture == fontTexture);
    CHECK(shownPixels(PixelFont::makeAtlas(), sprites[0]) == R"(
......
......
.####.
#...#.
#...#.
.####.
....#.
.###..
)");
}

TEST_CASE("PixelFont places the characters cell by cell at the given scale and color") {
    std::vector<Sprite> sprites;
    PixelFont(fontTexture).appendText(sprites, "Hi!", {10.0f, 20.0f}, 2, orange);
    REQUIRE(sprites.size() == 3);
    CHECK(sprites[0].position == Vec2{10.0f, 20.0f});
    CHECK(sprites[1].position == Vec2{22.0f, 20.0f});
    CHECK(sprites[2].position == Vec2{34.0f, 20.0f});
    CHECK(sprites[2].size == Vec2{12.0f, 16.0f});
    CHECK(sprites[2].color.g == orange.g);
}

TEST_CASE("PixelFont rounds the position to whole pixels, so that the glyphs stay sharp") {
    std::vector<Sprite> sprites;
    PixelFont(fontTexture).appendText(sprites, "A", {10.4f, 20.6f}, 1, orange);
    REQUIRE(sprites.size() == 1);
    CHECK(sprites[0].position == Vec2{10.0f, 21.0f});
}

TEST_CASE("PixelFont reads UTF-8 and shows characters without a glyph as a box") {
    const std::vector<Sprite> sprites = textSprites("ä€\xFF");
    REQUIRE(sprites.size() == 3);
    const etude::Image atlas = PixelFont::makeAtlas();
    CHECK(shownPixels(atlas, sprites[0]) == R"(
.#.#..
......
.###..
....#.
.####.
#...#.
.####.
......
)");
    CHECK(shownPixels(atlas, sprites[1]) == R"(
#####.
#...#.
#...#.
#...#.
#...#.
#...#.
#####.
......
)");
    CHECK(sprites[2].uv == sprites[1].uv);
}

TEST_CASE("PixelFont measures a text by its characters, not by its bytes") {
    CHECK(PixelFont::measure("Häschen", 3) == Vec2{126.0f, 24.0f});
    CHECK(PixelFont::measure("", 1) == Vec2{0.0f, 8.0f});
}

TEST_CASE("PixelFont keeps the last column of every cell empty, so that neighboring glyphs never touch") {
    const etude::Image atlas = PixelFont::makeAtlas();
    int setPixels = 0;
    for (int y = 0; y < atlas.height; ++y) {
        for (int x = PixelFont::cellWidth - 1; x < atlas.width; x += PixelFont::cellWidth) {
            setPixels += atlas.pixels[static_cast<std::size_t>(y * atlas.width + x)].a != 0 ? 1 : 0;
        }
    }
    CHECK(setPixels == 0);
}
