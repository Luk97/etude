#pragma once

namespace etude::ui {

    /// @brief How an element finds its size along one axis: as a fixed number of pixels, as a share of the element
    /// around it, as large as its text or as large as its children, the padding always included.
    struct SizeRule {
        enum class Kind {
            Pixels,
            Share,
            Text,
            Children
        };

        Kind kind = Kind::Children;

        /// @brief The number of pixels or the share, for the two kinds that have one.
        float value = 0.0f;

        /// @brief A fixed number of pixels of the screen, already multiplied by the scale.
        static constexpr SizeRule pixels(float amount) {
            return {
                .kind = Kind::Pixels,
                .value = amount,
            };
        }

        /// @brief A share of the space inside the padding of the element around, from 0 to 1. The size of the element
        /// around must not follow its children on this axis, because such a size is not known yet when the share is
        /// taken.
        static constexpr SizeRule share(float part) {
            return {
                .kind = Kind::Share,
                .value = part,
            };
        }

        /// @brief The size of the text plus the padding on both sides.
        static constexpr SizeRule text() {
            return {
                .kind = Kind::Text,
            };
        }

        /// @brief Just large enough for the children plus the padding: along the direction in which they follow each
        /// other the sum of their sizes and the spacing between them, across it and on both axes of a box the largest
        /// of them.
        static constexpr SizeRule children() {
            return {
                .kind = Kind::Children,
            };
        }
    };
}
