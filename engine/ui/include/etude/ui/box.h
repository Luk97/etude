#pragma once

#include <etude/math/color.h>
#include <etude/math/rect.h>

#include <optional>
#include <string_view>

namespace etude::ui {

    /// @brief One of the two directions in which boxes line up.
    enum class Axis {
        X,
        Y
    };

    /// @brief How a box finds its size along one axis. Pixels and padding count at scale 1, the context multiplies
    /// them with its scale.
    struct SizeRule {

        enum class Kind {
            Pixels,
            Text,
            ParentShare,
            Children
        };

        Kind kind = Kind::Children;
        float value = 0.0f;

        /// @brief A fixed number of pixels.
        static constexpr SizeRule pixels(float amount) {
            return {
                .kind = Kind::Pixels,
                .value = amount,
            };
        }

        /// @brief The size of the visible text plus the padding on both sides.
        static constexpr SizeRule fitText(float padding) {
            return {
                .kind = Kind::Text,
                .value = padding,
            };
        }

        /// @brief A share of the size of the parent, from 0 to 1. A parent that fits its children does not know its
        /// size yet when its children ask, so there the share comes out as 0.
        static constexpr SizeRule parentShare(float share) {
            return {
                .kind = Kind::ParentShare,
                .value = share,
            };
        }

        /// @brief Just large enough for the children: the sum of their sizes along the axis in which they line up,
        /// and the largest of them across it.
        static constexpr SizeRule fitChildren() {
            return {
                .kind = Kind::Children,
            };
        }
    };

    /// @brief Describes a box for one frame: its name, its size, how its children line up, whether it reacts to the
    /// mouse and what it draws.
    struct BoxSpec {

        /// @brief Names the box and holds its text. A box without a label keeps nothing between frames, so it cannot
        /// be clickable.
        std::string_view label;

        /// @brief The text of the box as it is, instead of the visible part of the label, for text that may contain
        /// ## such as typed text.
        std::optional<std::string_view> text;

        SizeRule width;
        SizeRule height;

        /// @brief The axis along which the children of the box follow each other.
        Axis childAxis = Axis::Y;

        /// @brief Whether the box reacts to the mouse. Only the topmost clickable box under the mouse does.
        bool clickable = false;

        /// @brief Fills the box with this color, if set. A clickable box turns lighter under the mouse and darker while
        /// it is held.
        std::optional<Color> background;

        /// @brief Writes the text of the box in this color, if set, centered in the box.
        std::optional<Color> textColor;
    };

    /// @brief What the mouse does with a box in this frame. The mouse is tested against the rectangle of the last
    /// frame, because the rectangles of the current frame are only known after its layout.
    struct Signal {

        /// @brief The rectangle of the box in the last frame, in pixels from the top left corner of the window. Empty
        /// in the first frame of a box and for boxes without a label.
        Rect rect;

        /// @brief The mouse is over the box, and no other clickable box lies above it.
        bool hovered = false;

        /// @brief The left button went down over the box in this frame.
        bool pressed = false;

        /// @brief The left button went down over the box and is still down, wherever the mouse has moved since.
        bool held = false;

        /// @brief The left button came up over the box after it had gone down over it.
        bool clicked = false;
    };
}
