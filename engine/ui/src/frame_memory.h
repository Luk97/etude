#pragma once

#include "id.h"
#include "layer_state.h"
#include "scroll_state.h"

#include <cstddef>
#include <unordered_map>
#include <vector>

namespace etude::ui {

    /// @brief What the context keeps for its elements from one frame to the next: where the floating elements are and
    /// in which order, how far the scrolling elements are scrolled, and which element has the focus with its text
    /// cursor.
    ///
    /// The memory forgets nothing for now, so a panel that the game hides for a while comes back where it was.
    struct FrameMemory {
        /// @brief What every floating element keeps, by its ID.
        std::unordered_map<Id, LayerState> layers;

        /// @brief The floating elements from back to front.
        std::vector<Id> layerOrder;

        /// @brief How far every scrolling element is scrolled, by its ID.
        std::unordered_map<Id, ScrollState> scrolls;

        /// @brief The element that takes the keyboard, Id{} if none does.
        Id focus{};

        /// @brief Where the next typed character goes in the text of the element with the focus, in bytes.
        std::size_t cursor = 0;
    };
}
