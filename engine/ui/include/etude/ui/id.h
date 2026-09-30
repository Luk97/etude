#pragma once

#include <cstdint>
#include <string_view>

namespace etude::ui {

    /// @brief Names a box across frames, so that the UI finds its rectangle of the last frame and knows whether it is
    /// held.
    enum class Id : std::uint64_t {
    };

    /// @brief The ID of the box that covers the screen and holds all others. It is the start value of 64-bit FNV-1a,
    /// so the IDs of its children are plain FNV-1a hashes of their labels.
    inline constexpr Id rootId{0xcbf29ce484222325};

    /// @brief Hashes the label with 64-bit FNV-1a, starting from the ID of the parent, so that equal labels in
    /// different parents get different IDs. The whole label counts, including a part after ##.
    constexpr Id makeId(std::string_view label, Id parent) {
        constexpr std::uint64_t prime = 0x100000001b3;
        auto hash = static_cast<std::uint64_t>(parent);
        for (const char character : label) {
            hash ^= static_cast<unsigned char>(character);
            hash *= prime;
        }
        return Id{hash};
    }

    /// @brief Returns the part of the label that the box shows, which ends before ##. So "OK##2" shows as OK, but its
    /// ID differs from that of another OK.
    constexpr std::string_view visibleText(std::string_view label) {
        return label.substr(0, label.find("##"));
    }
}
