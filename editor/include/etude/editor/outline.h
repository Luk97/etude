#pragma once

#include <etude/math/color.h>
#include <etude/math/vec2.h>
#include <etude/rendering/sprite.h>

#include <array>

namespace etude::editor {

    /// @brief Returns the four edges of a rectangle as sprites of the white texture, each as thick as given and
    /// centered on its edge, for a frame around a sprite or the picture of a camera. The rectangle has its middle at
    /// the center and is turned clockwise by the rotation in degrees, like a Transform2D. A negative size counts like a
    /// positive one, so that a mirrored sprite gets the same frame.
    std::array<Sprite, 4> outline(Vec2 center, Vec2 size, float rotation, float thickness, Color color);
}
