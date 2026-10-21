#include <etude/editor/scene/editor_camera.h>

#include <algorithm>
#include <cmath>

namespace etude::editor {

    Camera2D EditorCamera::view(Vec2 viewport) const {
        return {
            .position = center - viewport * 0.5f / zoom,
            .zoom = zoom,
        };
    }

    Vec2 EditorCamera::toWorld(Vec2 point, Vec2 viewport) const {
        return center + (point - viewport * 0.5f) / zoom;
    }

    void EditorCamera::pan(Vec2 movement) {
        center = center - movement / zoom;
    }

    void EditorCamera::zoomAt(float notches, Vec2 point, Vec2 viewport) {
        // The point of the world under the mouse has to stay there, so the center moves with the new zoom.
        const Vec2 held = toWorld(point, viewport);
        zoom = std::clamp(zoom * std::pow(zoomPerNotch, notches), minZoom, maxZoom);
        center = held - (point - viewport * 0.5f) / zoom;
    }
}
