#pragma once

#include <etude/math/color.h>
#include <etude/render2d/pixel_font.h>

/// @brief The look that the built-in widgets share, so that the widgets of a game can look the same: their colors, the
/// height of a line and the space between widgets, with lengths in pixels at scale 1.
namespace etude::ui::theme {

    inline constexpr Color textColor{
        .r = 0.9f,
        .g = 0.9f,
        .b = 0.9f,
    };

    inline constexpr Color buttonColor{
        .r = 0.25f,
        .g = 0.27f,
        .b = 0.33f,
    };

    /// @brief The color of what is on or filled, such as a ticked checkbox or the bar of a slider.
    inline constexpr Color accentColor{
        .r = 0.35f,
        .g = 0.55f,
        .b = 0.9f,
    };

    inline constexpr Color fieldColor{
        .r = 0.12f,
        .g = 0.13f,
        .b = 0.16f,
    };

    /// @brief The background of a panel, opaque, because with linear blending any transparency shows clearly.
    inline constexpr Color panelColor{
        .r = 0.08f,
        .g = 0.08f,
        .b = 0.11f,
    };

    inline constexpr Color titleColor{
        .r = 0.2f,
        .g = 0.3f,
        .b = 0.5f,
    };

    /// @brief The padding around a line of text, so that all widgets are equally high.
    inline constexpr float linePadding = 3.0f;

    /// @brief The height of a widget: a line of text with its padding.
    inline constexpr float lineHeight = PixelFont::cellHeight + 2.0f * linePadding;

    /// @brief The space between the widgets of a panel or of a row of buttons.
    inline constexpr float gap = 3.0f;

    /// @brief Returns the color of a clickable element: lighter while the mouse is over it, darker while it is held. A
    /// clickable element gets it for its background by itself, and widgets call it for parts that light up with the
    /// element around them, such as the square of a checkbox.
    Color highlight(Color color, bool hovered, bool held);
}
