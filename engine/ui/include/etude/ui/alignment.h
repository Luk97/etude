#pragma once

namespace etude::ui {

    /// @brief Where children sit in the space that they leave free in their layout: at its start, in its middle or at
    /// its end, where the start is the left or the top.
    ///
    /// A box applies the alignment along both axes, because its children lie on top of each other. A row applies it
    /// from top to bottom and a column from left to right, because along their own direction the children follow each
    /// other and leave no space to share.
    enum class Alignment {
        Start,
        Center,
        End
    };
}
