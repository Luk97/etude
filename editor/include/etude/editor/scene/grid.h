#pragma once

#include <etude/editor/scene/editor_camera.h>
#include <etude/math/vec2.h>
#include <etude/rendering/sprite.h>

#include <vector>

namespace etude::editor {

    /// @brief The distance between the lines of the grid in units of the world.
    inline constexpr float gridSpacing = 32.0f;

    /// @brief How close the lines of the grid may come on the screen, in pixels.
    inline constexpr float minGridGap = 16.0f;

    /// @brief Returns the lines of the grid that the camera shows in a viewport of the given size, as sprites of the
    /// white texture in world coordinates, each one pixel wide: one line every gridSpacing units, or every two, four
    /// and so on, as far as needed to keep them minGridGap pixels apart. The lines through the origin are lighter, so
    /// that the axes stand out.
    std::vector<Sprite> gridLines(const EditorCamera& camera, Vec2 viewport);
}
