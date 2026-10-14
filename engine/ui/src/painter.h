#pragma once

#include "hit_area.h"
#include "id.h"
#include "node.h"

#include <etude/math/rect.h>
#include <etude/render2d/pixel_font.h>
#include <etude/rendering/draw_batch.h>
#include <etude/rendering/sprite.h>

#include <cstddef>
#include <optional>
#include <span>
#include <unordered_map>
#include <vector>

namespace etude::ui {

    /// @brief Turns the nodes of a frame into sprites and batches and keeps this picture until the next frame, along
    /// with where the elements ended up.
    ///
    /// The next frame tests the mouse against the picture: against the rectangles of the elements with a key, the
    /// elements that take clicks or scroll, and the parts of the screen that the UI covers. Everything outside of
    /// floating elements is drawn first, the floating elements follow from back to front, so the one in front covers
    /// the others. Children only draw inside the clip rectangle of their parents, and sprites that follow each other
    /// with the same clip rectangle share a batch.
    class Painter {
    public:
        /// @brief What paint draws: the nodes after their layout, the indices of the floating ones, their order from
        /// back to front, the hot and the active element, and the scale of the text.
        struct Frame {
            std::span<Node> nodes;
            std::span<const std::size_t> floating;
            std::span<const Id> layerOrder;
            Id hot;
            Id active;
            int scale = 1;
        };

        /// @brief Writes with the font, whose atlas the renderer already holds.
        explicit Painter(PixelFont font);

        /// @brief Draws the nodes, parents before their children: first everything outside of floating elements, then
        /// the floating elements from back to front. Clickable elements turn lighter while hot and darker while active.
        void paint(const Frame& frame);

        /// @brief Returns the sprites of the last picture in drawing order, in pixels of the screen, without their clip
        /// rectangles.
        std::span<const Sprite> sprites() const;

        /// @brief Returns the sprites of the last picture as batches for the renderer, in drawing order, each with the
        /// matrix of the screen and the clip rectangle of its elements.
        std::span<const DrawBatch> batches() const;

        /// @brief Returns the rectangle of the element with this ID in the last picture, or an empty one if the picture
        /// holds no such element with a key.
        Rect rectOf(Id id) const;

        /// @brief Returns the clickable elements of the last picture in drawing order, so the last one under the mouse
        /// is on top.
        std::span<const HitArea> clickables() const;

        /// @brief Returns the scrolling elements of the last picture in drawing order, so the last one under the mouse
        /// is the innermost.
        std::span<const HitArea> scrollables() const;

        /// @brief Returns the visible parts of the elements that cover the screen in the last picture: those that drew
        /// something, and those that take the mouse or scroll even without drawing.
        std::span<const Rect> covered() const;

    private:
        /// @brief Sprites that follow each other in the draw list and share a clip rectangle.
        struct BatchRange {
            std::optional<Rect> clip;
            std::size_t first = 0;
            std::size_t count = 0;
        };

        /// @brief Draws the nodes from first up to end, parents before their children, and jumps over the subtrees of
        /// floating nodes if asked.
        void drawNodes(const Frame& frame, std::size_t first, std::size_t end, bool skipFloating);

        /// @brief Draws one node within the clip rectangle of its parent and keeps where it ended up.
        void drawNode(const Frame& frame, std::size_t index);

        PixelFont font;
        std::vector<Sprite> drawList;

        /// @brief How the draw list splits into batches, found while drawing.
        std::vector<BatchRange> ranges;

        std::vector<DrawBatch> drawBatches;

        /// @brief The rectangles of the elements with a key, for rectOf.
        std::unordered_map<Id, Rect> lastRects;

        std::vector<HitArea> lastClickables;
        std::vector<HitArea> lastScrollables;

        /// @brief The visible parts of the elements that cover the screen, for covered.
        std::vector<Rect> lastCovered;
    };
}
