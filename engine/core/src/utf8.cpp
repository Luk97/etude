#include <etude/core/utf8.h>

namespace etude {

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
}
