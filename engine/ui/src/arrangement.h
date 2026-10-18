#pragma once

#include <cstddef>
#include <optional>
#include <utility>

namespace etude::ui {

    /// @brief How a layout lines up its children: on top of each other, as box does, from left to right, as row does,
    /// or from top to bottom, as column does.
    enum class Arrangement {
        Stack,
        Row,
        Column
    };

    /// @brief Returns the axis along which a layout of this arrangement lines up its children, 0 for x and 1 for y, or
    /// none if they lie on top of each other.
    constexpr std::optional<std::size_t> mainAxis(Arrangement arrangement) {
        switch (arrangement) {
            case Arrangement::Stack:
                return std::nullopt;
            case Arrangement::Row:
                return 0;
            case Arrangement::Column:
                return 1;
        }
        std::unreachable();
    }
}
