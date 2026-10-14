#pragma once

namespace etude::ui {

    /// @brief How a layout lines up its children: on top of each other, as box does, from left to right, as row does,
    /// or from top to bottom, as column does.
    enum class Arrangement {
        Stack,
        Row,
        Column
    };
}
