#pragma once

#include <etude/core/image.h>
#include <etude/math/color.h>
#include <etude/math/vec2.h>
#include <etude/rendering/sprite.h>
#include <etude/rendering/texture_id.h>

#include <string_view>
#include <vector>

namespace etude {

    /// @brief The pixel font built into ETUDE, for printable ASCII, ÄÖÜäöüß and É. Every character takes a cell of the
    /// same size: its glyph is five pixels wide and stands on the seventh row, descenders reach into the eights, and
    /// the sixth column keeps neighboring glyphs apart. Characters without a glyph show as a box.
    class PixelFont {
    public:
        /// @brief Width of a character in pixels at scale 1.
        static constexpr int cellWidth = 6;

        /// @brief Height of a character cell in pixels at scale 1, which is also the height of a line.
        static constexpr int cellHeight = 8;

        /// @brief Draws all glyphs white on transparent into one image, from which the renderer makes the texture of
        /// the font.
        static Image makeAtlas();

        /// @brief Returns the size that the UTF-8 text covers at the given scale, one cell per character.
        static Vec2 measure(std::string_view text, int scale);

        /// @brief Draws from the texture that the renderer made from makeAtlas.
        explicit PixelFont(TextureId atlas);

        /// @brief Appends a sprite for each character of the UTF-8 text, from left to right, the first with its top
        /// left corner at the position. Rounds the position to whole pixels, so that the glyphs stay sharp at the whole
        /// scale.
        void appendText(
            std::vector<Sprite>& sprites,
            std::string_view text,
            Vec2 position,
            int scale,
            Color color
        ) const;

    private:
        TextureId atlas;
    };
}
