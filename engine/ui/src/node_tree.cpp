#include "node_tree.h"

#include <etude/core/assert.h>

#include <utility>

namespace etude::ui {

    void NodeTree::beginFrame(Size screen) {
        // Clearing keeps the memory of the lists, so once they have grown, they need no new memory.
        frameNodes.clear();
        floatingIndices.clear();

        // The root keeps the size of the screen. Its rules only say that it does not fit its children.
        frameNodes.push_back({
            .id = rootId,
            .rules = {SizeRule::pixels(0.0f), SizeRule::pixels(0.0f)},
            .size = {static_cast<float>(screen.width), static_cast<float>(screen.height)},
        });

        // Only the root is open, so the first nodes become its children.
        openNodes.assign(1, 0);
    }

    void NodeTree::endFrame() {
        // Only the root may still be open, and its subtree holds all nodes of the frame.
        ETUDE_ASSERT(openNodes.size() == 1);
        frameNodes.front().end = frameNodes.size();
    }

    Id NodeTree::open(std::string_view key, Node node) {
        // The node that was opened last becomes the parent. An empty key leaves the hash of the parent as it is, so the
        // children of a node without a key are named as if they belonged to its parent.
        const std::size_t parent = openNodes.back();
        node.id = makeId(key, frameNodes[parent].id);
        node.keyed = !key.empty();
        node.parent = parent;

        // A floating node is a layer of its own, any other node lies in the layer of its parent.
        node.layer = node.floating ? node.id : frameNodes[parent].layer;

        // Floating nodes go into a list of their own, from which the context places them and the painter draws them on
        // top. Only the other children follow each other and count towards the gaps of their parent.
        if (node.floating) {
            floatingIndices.push_back(frameNodes.size());
        } else {
            ++frameNodes[parent].childCount;
        }

        // The new node stays open until close, so the next nodes become its children.
        frameNodes.push_back(std::move(node));
        openNodes.push_back(frameNodes.size() - 1);
        return frameNodes.back().id;
    }

    void NodeTree::close() {
        // The root stays open until endFrame. The subtree of the closed node ends with the last node added so far.
        ETUDE_ASSERT(openNodes.size() > 1);
        frameNodes[openNodes.back()].end = frameNodes.size();
        openNodes.pop_back();
    }

    std::size_t NodeTree::current() const {
        return openNodes.back();
    }

    Node& NodeTree::node(std::size_t index) {
        return frameNodes[index];
    }

    std::span<Node> NodeTree::nodes() {
        return frameNodes;
    }

    std::span<const std::size_t> NodeTree::floatingNodes() const {
        return floatingIndices;
    }
}
