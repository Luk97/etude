#pragma once

#include "node.h"

#include <span>

namespace etude::ui {

    /// @brief Computes the size and position of every node. All lengths arrive in pixels of the screen, so the scale
    /// only sets how large the texts are.
    ///
    /// The nodes lie in the order of building, and the root at index 0 keeps its size. Per axis, the sizes come in
    /// three passes: first everything that needs nothing but the node itself, then shares of the parent from the root
    /// down, then the nodes that fit their children from the leaves up. The positions follow in one pass from the root
    /// down, and with them how large the children of every node are together. Floating nodes sit at their offset and
    /// take no space from their siblings.
    void layout(std::span<Node> nodes, int scale);
}
