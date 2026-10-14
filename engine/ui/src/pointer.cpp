#include "pointer.h"

#include "painter.h"

#include <etude/math/rect.h>
#include <etude/math/vec2.h>

#include <algorithm>

namespace etude::ui {

    namespace {

        /// @brief Returns the ID of the last area under the mouse that lies in the given floating element, or Id{} if
        /// there is none.
        Id lastUnder(std::span<const HitArea> areas, Vec2 mouse, Id layer) {
            for (auto area = areas.rbegin(); area != areas.rend(); ++area) {
                if (area->layer == layer && area->visible.contains(mouse)) {
                    return area->id;
                }
            }
            return Id{};
        }
    }

    void Pointer::beginFrame(const Input& input, const Painter& picture, std::span<const Id> layerOrder) {
        const Vec2 mouse = input.mousePosition();

        // The floating element in front under the mouse covers everything below it, so only its own elements can react.
        topLayerId = Id{};
        for (auto id = layerOrder.rbegin(); id != layerOrder.rend(); ++id) {
            if (picture.rectOf(*id).contains(mouse)) {
                topLayerId = *id;
                break;
            }
        }

        // The clickable element drawn last lies on top and becomes hot. While an element is held, no other element can
        // become hot.
        hotId = lastUnder(picture.clickables(), mouse, topLayerId);
        if (activeId != Id{} && hotId != activeId) {
            hotId = Id{};
        }

        // A scrolling element inside another is drawn after it, so the last one under the mouse is the innermost.
        scrollTargetId = lastUnder(picture.scrollables(), mouse, topLayerId);

        // A press belongs to where it began, up to and including the frame in which the button comes up, to the UI if
        // it began over what the UI covers. Without a press, the mouse belongs to the UI while it is over what the UI
        // covers.
        const bool overUi =
            std::ranges::any_of(picture.covered(), [&](const Rect& rect) { return rect.contains(mouse); });
        if (input.pressed(MouseButton::Left)) {
            pressOnUi = overUi;
        }
        const bool pressing = input.held(MouseButton::Left) || input.released(MouseButton::Left);
        mouseWanted = pressing ? pressOnUi : overUi;
    }

    Interaction Pointer::interact(Id id, const Input& input) {
        // A press on the hot element makes it the active one, which keeps the mouse until the button comes up.
        const bool hovered = hotId == id;
        const bool pressed = hovered && input.pressed(MouseButton::Left);
        if (pressed) {
            activeId = id;
        }

        // A click needs the press and the release over the same element.
        return {
            .hovered = hovered,
            .pressed = pressed,
            .held = activeId == id && input.held(MouseButton::Left),
            .clicked = activeId == id && hovered && input.released(MouseButton::Left),
        };
    }

    void Pointer::endFrame(const Input& input) {
        // Once the left button is up, no element is held any more, even if the held element has disappeared. This only
        // happens at the end of the frame, so that the frame in which the button comes up still counts the click.
        if (!input.held(MouseButton::Left)) {
            activeId = Id{};
        }
    }

    Id Pointer::hot() const {
        return hotId;
    }

    Id Pointer::active() const {
        return activeId;
    }

    Id Pointer::topLayer() const {
        return topLayerId;
    }

    Id Pointer::scrollTarget() const {
        return scrollTargetId;
    }

    bool Pointer::wantsMouse() const {
        return mouseWanted;
    }
}
