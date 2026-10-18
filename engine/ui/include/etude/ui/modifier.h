#pragma once

#include <etude/math/color.h>
#include <etude/math/vec2.h>
#include <etude/ui/alignment.h>

#include <array>
#include <optional>
#include <string>
#include <string_view>

namespace etude::ui {

    /// @brief Describes how an element looks and behaves: its key, its size, the space around and between its children,
    /// its background and how it reacts to the mouse and the keyboard.
    ///
    /// A modifier is built as a chain that starts with Modifier(), for example
    /// Modifier().key("OK").padding(4.0f).clickable(). Each call returns a copy with one more setting, so a modifier
    /// can be kept in a variable and be extended later. Unlike in Jetpack Compose, the order of the calls does not
    /// matter: each setting has its own place, a later call of the same kind replaces an earlier one, and a fixed
    /// length wins over a share. Whatever a modifier leaves unset, the building block decides, so a text keeps the
    /// padding of a line and a layout fits its children. Lengths count in pixels at scale 1, which the context
    /// multiplies by its scale.
    class Modifier {
    public:
        /// @brief Names the element, so that it can react to the mouse and keep its state from one frame to the next. A
        /// name builds on the names of the elements around it that have a key, so equal names in differently named
        /// layouts do not clash, while an element without a key adds nothing to the names of its children. Clickable,
        /// focusable, scrolling and floating elements and drag handles need a key.
        [[nodiscard]] Modifier key(std::string_view name) const;

        /// @brief Makes the element this many pixels wide, instead of as wide as its children or its text.
        [[nodiscard]] Modifier width(float pixels) const;

        /// @brief Makes the element this many pixels high, instead of as high as its children or its text.
        [[nodiscard]] Modifier height(float pixels) const;

        /// @brief Makes the element this many pixels wide and high.
        [[nodiscard]] Modifier size(float width, float height) const;

        /// @brief Makes the element this share of the width inside the padding of the element around it, from 0 to 1.
        /// That element needs a width that does not follow its children, such as a fixed width or a share of its own,
        /// because a width that follows the children is not known yet when the share is taken.
        [[nodiscard]] Modifier fillMaxWidth(float share = 1.0f) const;

        /// @brief Makes the element this share of the height inside the padding of the element around it, from 0 to 1,
        /// like fillMaxWidth.
        [[nodiscard]] Modifier fillMaxHeight(float share = 1.0f) const;

        /// @brief Makes the element this share of both the width and the height inside the padding of the element
        /// around it, like fillMaxWidth and fillMaxHeight.
        [[nodiscard]] Modifier fillMaxSize(float share = 1.0f) const;

        /// @brief Gives the element a part of the space that its siblings leave free along the direction of the row or
        /// the column around it, in proportion to the weights of all siblings that have one. Siblings with a fixed
        /// length, a share or a text count with their length, while siblings that fit their children only count with
        /// their padding and spacing, because their length is not known yet. Across the direction and inside a box, the
        /// weight changes nothing. A fixed length along the direction wins over the weight, and the weight wins over a
        /// share.
        [[nodiscard]] Modifier weight(float part = 1.0f) const;

        /// @brief Keeps the children or the text this far from the edges of the element, the same on all sides.
        [[nodiscard]] Modifier padding(float pixels) const;

        /// @brief Keeps the children or the text this far from the edges of the element, one length for the left and
        /// the right and one for the top and the bottom.
        [[nodiscard]] Modifier padding(float horizontal, float vertical) const;

        /// @brief Keeps this much space between children that follow each other in a row or a column.
        [[nodiscard]] Modifier spacedBy(float pixels) const;

        /// @brief Sets where the children sit in the space that they leave free in the element.
        [[nodiscard]] Modifier contentAlignment(Alignment alignment) const;

        /// @brief Fills the element with this color. A clickable element turns lighter under the mouse and darker while
        /// it is held.
        [[nodiscard]] Modifier background(Color color) const;

        /// @brief Shows the text of the element in this color instead of the text color of the theme.
        [[nodiscard]] Modifier textColor(Color color) const;

        /// @brief Cuts off whatever the children draw outside the element, and the mouse misses what is cut off.
        [[nodiscard]] Modifier clip() const;

        /// @brief Lets the element react to the mouse. Only the topmost clickable element under the mouse does, and a
        /// click only counts when the left button goes down and comes up over the element.
        [[nodiscard]] Modifier clickable() const;

        /// @brief Lets a press on the element give it the focus, so that it takes the keyboard until a press elsewhere,
        /// Enter, Escape or a frame that no longer builds the element. A focusable element is clickable as well.
        [[nodiscard]] Modifier focusable() const;

        /// @brief Lets the wheel scroll the children up and down while the mouse is over the element, as far as they
        /// reach beyond its bottom. Of scrolling elements inside each other, the innermost one under the mouse scrolls.
        /// The element should clip as well, so that nothing shows outside of it.
        [[nodiscard]] Modifier verticalScroll() const;

        /// @brief Lets the element float above everything else. It starts at this position, in pixels at scale 1 from
        /// the top left corner of the screen, and stays wherever a drag handle moves it. A press on a floating element
        /// brings it to the front, and the one in front covers those behind it. Floating elements are only allowed at
        /// the top level, where they take no space from the other elements.
        [[nodiscard]] Modifier floating(Vec2 start) const;

        /// @brief Lets dragging this element move the floating element that it lies in, as the title of a panel does.
        /// The point where the mouse grabbed the handle stays on the screen, so the handle can always be reached again.
        /// A drag handle is clickable as well, and outside of floating elements it moves nothing.
        [[nodiscard]] Modifier dragHandle() const;

    private:
        friend class Context;

        /// @brief The settings of the chain, in a struct of their own because the calls above already use their names.
        /// Only the context reads them, when it adds the element. Lengths come horizontal first.
        struct Settings {
            /// @brief A copy of the key, so that a modifier in a variable may outlive the string it was named with.
            std::string key;

            /// @brief Fixed lengths, which win over shares.
            std::array<std::optional<float>, 2> pixels;

            /// @brief Shares of the element around.
            std::array<std::optional<float>, 2> shares;

            /// @brief The part of the free space along the direction of the layout around.
            std::optional<float> weight;

            /// @brief The padding, which keeps the default of the building block if unset.
            std::optional<std::array<float, 2>> padding;

            float spacing = 0.0f;
            Alignment alignment = Alignment::Start;
            std::optional<Color> background;
            std::optional<Color> textColor;
            bool clip = false;
            bool clickable = false;
            bool focusable = false;
            bool verticalScroll = false;

            /// @brief Where the element starts to float, if it floats.
            std::optional<Vec2> floating;

            bool dragHandle = false;
        };

        Settings settings;
    };
}
