#pragma once

#include <string_view>

namespace etude::ui {

    /// @brief Returns the part of a label that shows, which ends before ##. A widget takes the whole label as the key
    /// of its element and shows only this part, so "OK##2" shows as OK but is another element than OK.
    constexpr std::string_view visibleText(std::string_view label) {
        return label.substr(0, label.find("##"));
    }
}
