#include "wide_text.h"

#include <cstddef>

#include <windows.h>

namespace etude {

    std::wstring toWide(std::string_view text) {
        const int size = static_cast<int>(text.size());
        const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), size, nullptr, 0);
        std::wstring wide(static_cast<std::size_t>(length), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.data(), size, wide.data(), length);
        return wide;
    }
}
