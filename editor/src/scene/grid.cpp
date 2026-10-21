#include <etude/editor/scene/grid.h>

#include <etude/math/color.h>

#include <cmath>

namespace etude::editor {

    namespace {

        /// @brief The colors of the lines, opaque, because with linear blending any transparency shows clearly.
        constexpr Color lineColor{
            .r = 0.16f,
            .g = 0.16f,
            .b = 0.2f,
        };
        constexpr Color axisColor{
            .r = 0.32f,
            .g = 0.32f,
            .b = 0.4f,
        };
    }

    std::vector<Sprite> gridLines(const EditorCamera& camera, Vec2 viewport) {
        // The spacing doubles until the lines lie far enough apart on the screen.
        float spacing = gridSpacing;
        while (spacing * camera.zoom < minGridGap) {
            spacing *= 2.0f;
        }

        // The lines run across what the viewport shows, starting with the first multiple of the spacing at its left and
        // its top edge. Each is one pixel wide and centered on its multiple.
        const Vec2 start = camera.view(viewport).position;
        const Vec2 shown = viewport / camera.zoom;
        const float width = 1.0f / camera.zoom;
        std::vector<Sprite> lines;
        for (auto i = static_cast<int>(std::ceil(start.x / spacing)); i * spacing < start.x + shown.x; ++i) {
            lines.push_back({
                .position = {i * spacing - width / 2.0f, start.y},
                .size = {width, shown.y},
                .color = i == 0 ? axisColor : lineColor,
            });
        }
        for (auto i = static_cast<int>(std::ceil(start.y / spacing)); i * spacing < start.y + shown.y; ++i) {
            lines.push_back({
                .position = {start.x, i * spacing - width / 2.0f},
                .size = {shown.x, width},
                .color = i == 0 ? axisColor : lineColor,
            });
        }
        return lines;
    }
}
