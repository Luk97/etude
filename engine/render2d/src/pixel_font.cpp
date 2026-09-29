#include <etude/render2d/pixel_font.h>

#include "pixel_font_glyphs.h"

#include <etude/core/utf8.h>
#include <etude/math/rect.h>

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace etude {

    namespace {

        /// @brief With 16 x 8 cells, the texture coordinates of every cell are multiples of 1/16 and 1/8, which float
        /// stores exactly.
        constexpr int atlasColumns = 16;
        constexpr int atlasRows = 8;

        static_assert(pixelFontGlyphs.size() <= atlasColumns * atlasRows, "The glyphs have to fit into the atlas.");
        static_assert(
            std::ranges::is_sorted(pixelFontGlyphs, {}, &GlyphArt::character),
            "The glyphs have to be sorted by character, so that cellOf can search them."
        );
        static_assert(
            pixelFontGlyphs.back().character == replacementCharacter,
            "The box for missing glyphs has to come last."
        );

        /// @brief Returns the cell that holds the glyph of the character, or the cell of the box if there is none.
        std::size_t cellOf(char32_t character) {
            const auto glyph = std::ranges::lower_bound(pixelFontGlyphs, character, {}, &GlyphArt::character);
            if (glyph == pixelFontGlyphs.end() || glyph->character != character) {
                return pixelFontGlyphs.size() - 1;
            }
            return static_cast<size_t>(glyph - pixelFontGlyphs.begin());
        }

        /// @brief Returns where the cell lies in the atlas, in texture coordinates.
        Rect uvOf(std::size_t cell) {
            const auto column = static_cast<float>(cell % atlasColumns);
            const auto row = static_cast<float>(cell / atlasColumns);
            return {
                .position = {column / atlasColumns, row / atlasRows},
                .size = {1.0f / atlasColumns, 1.0f / atlasRows},
            };
        }
    }

    Image PixelFont::makeAtlas() {
        constexpr int width = atlasColumns * cellWidth;
        constexpr int height = atlasRows * cellHeight;
        constexpr Pixel white{
            .r = 255,
            .g = 255,
            .b = 255,
            .a = 255,
        };

        Image image{
            .width = width,
            .height = height,
            .pixels = std::vector<Pixel>(width * height),
        };
        for (std::size_t cell = 0; cell < pixelFontGlyphs.size(); ++cell) {
            const std::size_t left = (cell % atlasColumns) * cellWidth;
            const std::size_t top = (cell / atlasColumns) * cellHeight;
            const auto& rows = pixelFontGlyphs[cell].rows;
            for (std::size_t y = 0; y < rows.size(); ++y) {
                for (std::size_t x = 0; x < rows[y].size(); ++x) {
                    if (rows[y][x] == '#') {
                        image.pixels[(top + y) * width + left + x] = white;
                    }
                }
            }
        }
        return image;
    }

    Vec2 PixelFont::measure(std::string_view text, int scale) {
        int characters = 0;
        while (!text.empty()) {
            takeUtf8(text);
            ++characters;
        }
        return {static_cast<float>(characters * cellWidth * scale), static_cast<float>(cellHeight * scale)};
    }

    PixelFont::PixelFont(TextureId atlas) : atlas(atlas) {}

    void PixelFont::appendText(
        std::vector<Sprite>& sprites,
        std::string_view text,
        Vec2 position,
        int scale,
        Color color
    ) const {
        const Vec2 size{static_cast<float>(cellWidth * scale), static_cast<float>(cellHeight * scale)};
        Vec2 cursor{std::round(position.x), std::round(position.y)};
        while (!text.empty()) {
            sprites.push_back({
                .position = cursor,
                .size = size,
                .texture = atlas,
                .uv = uvOf(cellOf(takeUtf8(text))),
                .color = color,
            });
            cursor.x += size.x;
        }
    }
}
