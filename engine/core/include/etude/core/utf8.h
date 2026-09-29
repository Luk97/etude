#pragma once

#include <string>

namespace etude {

    /// @brief U+FFFD, which stands for characters that cannot be read or shown.
    inline constexpr char32_t replacementCharacter = U'\uFFFD';

    /// @brief Appends the character to the text in UTF-8, as one to four bytes.
    void appendUtf8(std::string& text, char32_t character);

    /// @brief Removes the first character from the UTF-8 text and returns it. A byte that does not start a valid
    /// character comes out as the replacement character, and only that byte is removed. The text must not be empty.
    char32_t takeUtf8(std::string_view& text);
}
