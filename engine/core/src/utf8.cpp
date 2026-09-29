#include <etude/core/utf8.h>

#include <array>
#include <cstddef>
#include <optional>

namespace etude {

    namespace {

        /// @brief A character and the number of bytes that it takes in UTF-8.
        struct Decoded {
            char32_t character = 0;
            std::size_t length = 0;
        };

        std::optional<Decoded> decode(std::string_view text) {
            const auto byteAt = [text](std::size_t index) { return static_cast<unsigned char>(text[index]); };
            const unsigned char lead = byteAt(0);
            if (lead < 0x80) {
                return Decoded{lead, 1};
            }

            // The lead byte tells the length and starts the character, each continuation byte adds six bits.
            std::size_t length = 0;
            char32_t character = 0;
            if ((lead & 0xE0) == 0xC0) {
                length = 2;
                character = lead & 0x1F;
            } else if ((lead & 0xF0) == 0xE0) {
                length = 3;
                character = lead & 0x0F;
            } else if ((lead & 0xF8) == 0xF0) {
                length = 4;
                character = lead & 0x07;
            } else {
                return std::nullopt;
            }
            if (text.size() < length) {
                return std::nullopt;
            }
            for (std::size_t i = 1; i < length; ++i) {
                if ((byteAt(i) & 0xC0) != 0x80) {
                    return std::nullopt;
                }
                character = (character << 6) | (byteAt(i) & 0x3F);
            }

            // Below the smallest character of each length, the  bytes are overlong.
            constexpr std::array<char32_t, 5> smallest{0, 0, 0x80, 0x800, 0x10000};
            const bool surrogate = character >= 0xD800 && character <= 0xDFFF;
            if (character < smallest[length] || character > 0x10FFFF || surrogate) {
                return std::nullopt;
            }
            return Decoded{character, length};
        }
    }

    void appendUtf8(std::string& text, char32_t character) {
        if (character < 0x80) {
            text += static_cast<char>(character);
        } else if (character < 0x800) {
            text += static_cast<char>(0xC0 | (character >> 6));
            text += static_cast<char>(0x80 | (character & 0x3F));
        } else if (character < 0x10000) {
            text += static_cast<char>(0xE0 | (character >> 12));
            text += static_cast<char>(0x80 | ((character >> 6) & 0x3F));
            text += static_cast<char>(0x80 | (character & 0x3F));
        } else {
            text += static_cast<char>(0xF0 | (character >> 18));
            text += static_cast<char>(0x80 | ((character >> 12) & 0x3F));
            text += static_cast<char>(0x80 | ((character >> 6) & 0x3F));
            text += static_cast<char>(0x80 | (character & 0x3F));
        }
    }

    char32_t takeUtf8(std::string_view& text) {
        const std::optional<Decoded> decoded = decode(text);
        if (!decoded) {
            text.remove_prefix(1);
            return replacementCharacter;
        }
        text.remove_prefix(decoded->length);
        return decoded->character;
    }
}
