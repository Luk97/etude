#pragma once

#include <cstdint>
#include <string_view>

namespace etude::ui {

    /// @brief Names an element across frames, so that the UI finds its rectangle of the last frame and knows whether it
    /// is held, has the focus or floats somewhere. Id{}, which is 0, stands for no element.
    enum class Id : std::uint64_t {
    };

    /// @brief The ID of the root, which covers the screen and holds all elements. It is the start value of 64-bit
    /// FNV-1a.
    inline constexpr Id rootId{0xcbf29ce484222325};

    /// @brief Hashes the key with 64-bit FNV-1a, starting from the ID of the parent, so that equal keys in different
    /// parents get different IDs. A byte that UTF-8 never uses goes before the key, so that keys that only join to the
    /// same text, such as 1 and 10 against 11 and 0, still differ. An empty key gives the ID of the parent itself.
    constexpr Id makeId(std::string_view key, Id parent) {
        if (key.empty()) {
            return parent;
        }

        // FNV-1a mixes in one byte after the other and multiplies by its prime, the separator first.
        constexpr std::uint64_t prime = 0x100000001b3;
        constexpr std::uint64_t separator = 0xFF;
        auto hash = static_cast<std::uint64_t>(parent);
        hash ^= separator;
        hash *= prime;
        for (const char character : key) {
            hash ^= static_cast<unsigned char>(character);
            hash *= prime;
        }
        return Id{hash};
    }
}
