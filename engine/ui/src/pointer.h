#pragma once

#include "id.h"

#include <etude/platform/input.h>
#include <etude/ui/interaction.h>

#include <span>

namespace etude::ui {

    class Painter;

    /// @brief Follows the mouse over the picture of the last frame: which clickable element it is over, which element
    /// it holds, which element the wheel scrolls and whether the mouse belongs to the UI or to the game.
    ///
    /// The floating element in front under the mouse covers everything below it, so only its own elements can react. An
    /// element on which the left button went down keeps the mouse until the button comes up, even when the mouse leaves
    /// it, and no other element reacts in the meantime.
    class Pointer {
    public:
        /// @brief Finds the floating element in front under the mouse, the clickable element on top under the mouse,
        /// which becomes hot unless another element is held, and the innermost scrolling element under the mouse. A
        /// press belongs to where it began, up to and including the frame in which the button comes up, to the UI if it
        /// began over what the UI covers.
        void beginFrame(const Input& input, const Painter& picture, std::span<const Id> layerOrder);

        /// @brief Returns what the mouse does with a clickable element in this frame. A press on the hot element makes
        /// it the active one.
        Interaction interact(Id id, const Input& input);

        /// @brief Lets go of the held element once the left button is up, even if the element has disappeared.
        void endFrame(const Input& input);

        Id hot() const;
        Id active() const;
        Id topLayer() const;
        Id scrollTarget() const;
        bool wantsMouse() const;

    private:
        /// @brief The clickable element under the mouse that may react in this frame, which its interaction calls
        /// hovered.
        Id hotId{};

        /// @brief The element on which the left button went down and that keeps the mouse until the button comes up,
        /// which its interaction calls held.
        Id activeId{};

        /// @brief The floating element in front under the mouse, which covers everything below it.
        Id topLayerId{};

        /// @brief The innermost scrolling element under the mouse, which the wheel scrolls.
        Id scrollTargetId{};

        /// @brief Whether the mouse belongs to the UI in this frame.
        bool mouseWanted = false;

        /// @brief Whether the last press of the left button began over what the UI covers.
        bool pressOnUi = false;
    };
}
