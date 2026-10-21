#pragma once

namespace etude::editor {

    /// @brief What the gizmo of the scene pane does with the selected entity: move it, turn it or stretch it.
    enum class GizmoMode {
        Move,
        Rotate,
        Scale
    };
}
