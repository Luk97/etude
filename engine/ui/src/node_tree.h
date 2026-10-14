#pragma once

#include "id.h"
#include "node.h"

#include <etude/math/size.h>

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace etude::ui {

    /// @brief The elements of one frame as a tree of nodes.
    ///
    /// The nodes lie in the order of building, so every parent comes before its children, and each new node becomes a
    /// child of the node that was opened last and is not closed yet. The tree names every node by its key, starting
    /// from the ID of its parent, and remembers the floating nodes, which the painter draws on top in their own order.
    class NodeTree {
    public:
        /// @brief Starts a frame with nothing but the root, which keeps the size of the screen and lines its children
        /// up from top to bottom.
        void beginFrame(Size screen);

        /// @brief Closes the root, once all other nodes are closed.
        void endFrame();

        /// @brief Adds the node as a child of the open node and opens it, so that the next nodes become its children
        /// until close. The key names the node, starting from the ID of the open node. Returns the ID of the node,
        /// which is that of the open node if the key is empty.
        Id open(std::string_view key, Node node);

        /// @brief Closes the node that open opened last.
        void close();

        /// @brief Returns the index of the node that was opened last and is not closed yet, 0 for the root.
        std::size_t current() const;

        /// @brief Returns the node at this index in the order of building.
        Node& node(std::size_t index);

        /// @brief Returns all nodes of the frame in the order of building, the root first.
        std::span<Node> nodes();

        /// @brief Returns the indices of the floating nodes in the order of building.
        std::span<const std::size_t> floatingNodes() const;

    private:
        std::vector<Node> frameNodes;

        /// @brief The indices of the nodes that open opened and close has not closed yet, the root first.
        std::vector<std::size_t> openNodes;

        std::vector<std::size_t> floatingIndices;
    };
}
