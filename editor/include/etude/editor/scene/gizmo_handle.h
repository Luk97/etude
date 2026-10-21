#pragma once

namespace etude::editor {

    /// @brief A part of the gizmo that the mouse can take: one of its two axes, the square in its middle or the ring
    /// for turning.
    enum class GizmoHandle {
        AxisX,
        AxisY,
        Center,
        Ring
    };
}
