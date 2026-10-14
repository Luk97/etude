#pragma once

#include "arrangement.h"
#include "id.h"
#include "size_rule.h"

#include <etude/math/color.h>
#include <etude/math/rect.h>
#include <etude/math/vec2.h>
#include <etude/ui/alignment.h>

#include <array>
#include <cstddef>
#include <optional>
#include <string>

namespace etude::ui {

    /// @brief An element of the current frame, with all that the parts of the context know about it, in pixels at the
    /// scale of the frame. The arrays hold one entry per axis, horizontal first.
    ///
    /// The nodes of a frame lie in a list in the order of building, so a node points to its parent by index and knows
    /// with end where its subtree stops. The fields come in groups, by who fills them in: the tree names the node and
    /// links it into the list, the context sets what the modifier said and what its memory keeps, the layout finds
    /// where the element goes, and the painter which clip rectangle applies to the children.
    struct Node {
        Id id;

        /// @brief Whether the element has a key of its own. Without one, it carries the ID of its parent and keeps
        /// nothing between frames.
        bool keyed = false;

        std::size_t parent = 0;

        /// @brief The floating element that the element lies in, which is itself for a floating element and Id{}
        /// outside of them.
        Id layer;

        /// @brief How many children follow each other in the element, without floating ones, which decides how many
        /// gaps lie between them.
        std::size_t childCount = 0;

        /// @brief One past the last node in the subtree.
        std::size_t end = 0;

        /// @brief The line that a text shows, empty for layouts.
        std::string text;

        std::array<SizeRule, 2> rules;
        Arrangement arrangement = Arrangement::Column;
        Alignment alignment = Alignment::Start;
        std::array<float, 2> padding{};
        float gap = 0.0f;
        bool clip = false;

        /// @brief Whether the element takes the mouse, as clickable and focusable elements and drag handles do.
        bool clickable = false;

        bool scrolls = false;
        std::optional<Color> background;

        /// @brief The color of the line, which only a text has.
        std::optional<Color> textColor;

        /// @brief Whether the element floats at its offset instead of following its previous sibling, so that it takes
        /// no space from its siblings.
        bool floating = false;

        /// @brief Where a floating element sits, measured from the top left corner of the screen, since floating
        /// elements only lie at the top level.
        Vec2 offset;

        /// @brief How far the children are moved up, for scrolling.
        float scroll = 0.0f;

        std::array<float, 2> position{};
        std::array<float, 2> size{};

        /// @brief Where the next child starts along the direction in which the children follow each other, measured
        /// from the padding.
        float nextChild = 0.0f;

        /// @brief How many children the layout has placed so far, so that a gap only goes between two of them.
        std::size_t placedChildren = 0;

        /// @brief How large the children are together, without the padding: along the direction in which they follow
        /// each other their sizes and the gaps, across it the largest of them.
        std::array<float, 2> contentSize{};

        /// @brief The clip rectangle that applies to the children, found while drawing.
        std::optional<Rect> childClip;

        /// @brief Returns the rectangle that the layout found, in pixels of the screen.
        Rect rect() const {
            return {
                .position = {position[0], position[1]},
                .size = {size[0], size[1]},
            };
        }
    };
}
