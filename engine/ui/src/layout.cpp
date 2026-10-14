#include "layout.h"

#include <etude/math/vec2.h>
#include <etude/render2d/pixel_font.h>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>

namespace etude::ui {

    namespace {

        /// @brief The indices of the axes in the arrays of a node.
        constexpr std::size_t horizontal = 0;
        constexpr std::size_t vertical = 1;

        /// @brief Returns the axis along which the children of the node follow each other, or none if they lie on top
        /// of each other.
        std::optional<std::size_t> mainAxis(const Node& node) {
            switch (node.arrangement) {
                case Arrangement::Stack:
                    return std::nullopt;
                case Arrangement::Row:
                    return horizontal;
                case Arrangement::Column:
                    return vertical;
            }
            std::unreachable();
        }

        /// @brief Returns how far the alignment moves a child into the space that it leaves free.
        float aligned(Alignment alignment, float freeSpace) {
            switch (alignment) {
                case Alignment::Start:
                    return 0.0f;
                case Alignment::Center:
                    return freeSpace * 0.5f;
                case Alignment::End:
                    return freeSpace;
            }
            std::unreachable();
        }
    }

    void layout(std::span<Node> nodes, int scale) {
        // Widths and heights do not depend on each other, so each axis gets its own three passes over the sizes.
        for (const std::size_t axis : {horizontal, vertical}) {
            // Sizes that need nothing but the node itself. A node that fits its children starts with its padding and
            // the gaps between them. The root at index 0 keeps the size of the screen.
            for (std::size_t i = 1; i < nodes.size(); ++i) {
                Node& node = nodes[i];
                const SizeRule rule = node.rules[axis];
                switch (rule.kind) {
                    case SizeRule::Kind::Pixels:
                        node.size[axis] = rule.value;
                        break;
                    case SizeRule::Kind::Share:
                        // A share follows in the next pass, once the parent has its size.
                        break;
                    case SizeRule::Kind::Text: {
                        const Vec2 textSize = PixelFont::measure(node.text, scale);
                        node.size[axis] = (axis == horizontal ? textSize.x : textSize.y) + 2.0f * node.padding[axis];
                        break;
                    }
                    case SizeRule::Kind::Children: {
                        const std::size_t gaps =
                            mainAxis(node) == axis && node.childCount > 1 ? node.childCount - 1 : 0;
                        node.size[axis] = 2.0f * node.padding[axis] + static_cast<float>(gaps) * node.spacing;
                        break;
                    }
                }
            }

            // Shares of the space inside the parent, which comes earlier in the list and so has its size already.
            for (std::size_t i = 1; i < nodes.size(); ++i) {
                Node& node = nodes[i];
                if (node.rules[axis].kind == SizeRule::Kind::Share) {
                    const Node& parent = nodes[node.parent];
                    node.size[axis] = (parent.size[axis] - 2.0f * parent.padding[axis]) * node.rules[axis].value;
                }
            }

            // Parents that fit their children, which floating children leave alone. Walking backwards meets all
            // children of a node before the node itself. Along the direction in which the children follow each other,
            // their sizes add up, across it the largest of them decides.
            for (std::size_t i = nodes.size() - 1; i > 0; --i) {
                const Node& child = nodes[i];
                Node& parent = nodes[child.parent];
                if (child.floating || parent.rules[axis].kind != SizeRule::Kind::Children) {
                    continue;
                }
                parent.size[axis] = mainAxis(parent) == axis
                                        ? parent.size[axis] + child.size[axis]
                                        : std::max(parent.size[axis], child.size[axis] + 2.0f * parent.padding[axis]);
            }
        }

        // The positions follow from the root down, so the parent of every node has its own already.
        for (std::size_t i = 1; i < nodes.size(); ++i) {
            Node& node = nodes[i];
            Node& parent = nodes[node.parent];

            // A floating node sits at its offset and leaves the places of its siblings alone.
            if (node.floating) {
                node.position = {parent.position[0] + node.offset.x, parent.position[1] + node.offset.y};
                continue;
            }

            // Every child starts inside the padding of its parent. Along the direction of a row or a column, it follows
            // its previous sibling after the spacing. Across it, and on both axes of a box, it sits where the alignment
            // puts it in the space that it leaves free. On the way, the parent learns how large its children are
            // together.
            const std::optional<std::size_t> along = mainAxis(parent);
            for (const std::size_t axis : {horizontal, vertical}) {
                const float start = parent.position[axis] + parent.padding[axis];
                if (along == axis) {
                    if (parent.placedChildren > 0) {
                        parent.nextChild += parent.spacing;
                    }
                    node.position[axis] = start + parent.nextChild;
                    parent.nextChild += node.size[axis];
                    parent.contentSize[axis] = parent.nextChild;
                } else {
                    const float freeSpace = parent.size[axis] - 2.0f * parent.padding[axis] - node.size[axis];
                    node.position[axis] = start + aligned(parent.alignment, freeSpace);
                    parent.contentSize[axis] = std::max(parent.contentSize[axis], node.size[axis]);
                }
            }

            // Scrolling moves the child up, and with it everything inside, whose positions build on its own.
            node.position[vertical] -= parent.scroll;
            ++parent.placedChildren;
        }
    }
}
