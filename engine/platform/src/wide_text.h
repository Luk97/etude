#pragma once

#include <string>
#include <string_view>

namespace etude {

    /// @brief Converts UTF-8 to UTF-16, the encoding of the wide-character Win32 functions.
    std::wstring toWide(std::string_view text);
}
