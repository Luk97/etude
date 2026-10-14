#include "painter.h"

#include <etude/math/mat3.h>
#include <etude/math/vec2.h>
#include <etude/ui/theme.h>

#include <algorithm>

namespace etude::ui {

    namespace {

        /// @brief Returns the part of the rectangle inside the clip rectangle, or all of it without one.
        Rect visiblePart(Rect rect, const std::optional<Rect>& clip) {
            return clip ? clip->intersection(rect) : rect;
        }
    }

    Painter::Painter(PixelFont font) : font(font) {}

    void Painter::paint(const Frame& frame) {
        // The new picture replaces the last one, while the lists keep their memory.
        drawList.clear();
        ranges.clear();
        drawBatches.clear();
        lastRects.clear();
        lastClickables.clear();
        lastScrollables.clear();
        lastCovered.clear();

        // First everything outside of floating elements, then each floating element with its subtree, from back to
        // front. Floating elements that this frame did not build any more are missing from the nodes and so skipped.
        drawNodes(frame, 0, frame.nodes.size(), true);
        for (const Id id : frame.layerOrder) {
            const auto isLayer = [&](std::size_t index) { return frame.nodes[index].id == id; };
            if (const auto layer = std::ranges::find_if(frame.floating, isLayer); layer != frame.floating.end()) {
                drawNodes(frame, *layer, frame.nodes[*layer].end, false);
            }
        }

        // Every range of sprites with one clip rectangle becomes a batch. The root covers the screen, so its rectangle
        // gives the matrix of all batches.
        const Mat3 projection = Mat3::orthographic(frame.nodes.front().rect());
        for (const BatchRange& range : ranges) {
            drawBatches.push_back({
                .viewProjection = projection,
                .clip = range.clip,
                .sprites = std::span<const Sprite>(drawList).subspan(range.first, range.count),
            });
        }
    }

    std::span<const Sprite> Painter::sprites() const {
        return drawList;
    }

    std::span<const DrawBatch> Painter::batches() const {
        return drawBatches;
    }

    Rect Painter::rectOf(Id id) const {
        const auto rect = lastRects.find(id);
        return rect != lastRects.end() ? rect->second : Rect{};
    }

    std::span<const HitArea> Painter::clickables() const {
        return lastClickables;
    }

    std::span<const HitArea> Painter::scrollables() const {
        return lastScrollables;
    }

    std::span<const Rect> Painter::covered() const {
        return lastCovered;
    }

    void Painter::drawNodes(const Frame& frame, std::size_t first, std::size_t end, bool skipFloating) {
        for (std::size_t i = first; i < end;) {
            // A floating node and its subtree wait for their turn among the layers.
            if (skipFloating && frame.nodes[i].floating) {
                i = frame.nodes[i].end;
                continue;
            }
            drawNode(frame, i);
            ++i;
        }
    }

    void Painter::drawNode(const Frame& frame, std::size_t index) {
        Node& node = frame.nodes[index];
        const Rect rect = node.rect();

        // The node draws inside the clip rectangle of its parent and, if it clips, passes on the part of itself that
        // stays visible. The root is its own parent, and it clips nothing.
        const std::optional<Rect> clip = frame.nodes[node.parent].childClip;
        node.childClip = node.clip ? std::optional<Rect>(visiblePart(rect, clip)) : clip;

        // An element with a key leaves its rectangle for the next frame, and its visible part if the mouse can click or
        // scroll it.
        if (node.keyed) {
            lastRects[node.id] = rect;
            const HitArea area{
                .id = node.id,
                .visible = visiblePart(rect, clip),
                .layer = node.layer,
            };
            if (node.clickable) {
                lastClickables.push_back(area);
            }
            if (node.scrolls) {
                lastScrollables.push_back(area);
            }
        }

        // The sprites of the node start here: the background, lighter under the mouse and darker while held if the
        // element is clickable, then the text in the middle.
        const std::size_t first = drawList.size();
        if (node.background) {
            const bool hovered = node.clickable && node.id == frame.hot;
            const bool held = node.clickable && node.id == frame.active;
            drawList.push_back({
                .position = rect.position,
                .size = rect.size,
                .color = theme::highlight(*node.background, hovered, held),
            });
        }
        if (node.textColor) {
            const Vec2 textSize = PixelFont::measure(node.text, frame.scale);
            const Vec2 textPosition = rect.position + (rect.size - textSize) * 0.5f;
            font.appendText(drawList, node.text, textPosition, frame.scale, *node.textColor);
        }

        // What drew something or takes the mouse covers the screen, so its visible part makes the UI want the mouse.
        const std::size_t count = drawList.size() - first;
        if (count > 0 || node.clickable || node.scrolls) {
            lastCovered.push_back(visiblePart(rect, clip));
        }
        if (count == 0) {
            return;
        }

        // Sprites that follow each other with the same clip rectangle share a batch.
        if (ranges.empty() || ranges.back().clip != clip) {
            ranges.push_back({
                .clip = clip,
                .first = first,
            });
        }
        ranges.back().count += count;
    }
}
