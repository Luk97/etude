#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace etude {

    /// @brief U+FFFD, which stands in for characters that cannot be read or shown.
    inline constexpr char32_t replacementCharacter = U'\uFFFD';

    /// @brief Appends the character to the text in UTF-8, as one to four bytes.
    void appendUtf8(std::string& text, char32_t character);

    /// @brief Removes the first character from the UTF-8 text and returns it. A byte that does not start a valid
    /// character comes out as the replacement character, and only that byte is removed. The text must not be empty.
    char32_t takeUtf8(std::string_view& text);

    /// @brief Returns where the character after the one at the offset starts, or the size of the text if that one is
    /// the last. The offset has to lie before the end of valid UTF-8 text.
    std::size_t nextCharacter(std::string_view text, std::size_t offset);

    /// @brief Returns where the character before the offset starts. The offset has to lie after the start of valid
    /// UTF-8 text.
    std::size_t previousCharacter(std::string_view text, std::size_t offset);
}
