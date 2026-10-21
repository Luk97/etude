#pragma once

#include <etude/math/vec2.h>
#include <etude/render2d/camera2d.h>

namespace etude::editor {

    /// @brief The camera of the scene pane, which the editor moves on its own, apart from the Camera of the scene, so
    /// that looking around changes nothing that the scene file keeps. Its center lies in the middle of the pane, and at
    /// zoom 1 one unit of the world covers one pixel.
    struct EditorCamera {
        /// @brief How far the camera zooms out and in: from a sixteenth of a pixel up to sixteen pixels per unit.
        static constexpr float minZoom = 1.0f / 16.0f;
        static constexpr float maxZoom = 16.0f;

        /// @brief How much one notch of the wheel zooms in.
        static constexpr float zoomPerNotch = 1.25f;

        Vec2 center;
        float zoom = 1.0f;

        /// @brief Returns the camera that shows the world this way in a viewport of the given size in pixels.
        Camera2D view(Vec2 viewport) const;

        /// @brief Returns the point of the world under a point of the viewport, in pixels from its top left corner.
        Vec2 toWorld(Vec2 point, Vec2 viewport) const;

        /// @brief Moves the camera so that the world follows the mouse, which moved by the given pixels.
        void pan(Vec2 movement);

        /// @brief Zooms in by the notches of the wheel, or out for negative ones, around a point of the viewport, which
        /// keeps showing the same point of the world. The zoom stays between minZoom and maxZoom.
        void zoomAt(float notches, Vec2 point, Vec2 viewport);
    };
}
