#pragma once

#include <string>

namespace etude {

    /// @brief Appends the character to the text in UTF-8, as one to four bytes.
    void appendUtf8(std::string& text, char32_t character);
}
