#pragma once

#include <etude/editor/scene/gizmo_handle.h>
#include <etude/editor/scene/gizmo_mode.h>
#include <etude/math/vec2.h>
#include <etude/rendering/sprite.h>
#include <etude/scene/components/transform2d.h>

#include <optional>
#include <vector>

namespace etude::editor {

    /// @brief The tool of the scene pane that moves, turns and stretches the selected entity with the mouse. Moving
    /// goes along the axes of the world, or freely with the square in the middle. Turning goes with a ring around the
    /// entity. Stretching goes along the axes of the entity, which turn with it, or evenly with the square. The parts
    /// keep their size on the screen at any zoom, and the points that the gizmo takes are in world coordinates.
    class Gizmo {
    public:
        /// @brief How long the axes are and how far the ring lies from the middle, in pixels of the screen.
        static constexpr float reach = 80.0f;

        /// @brief How large the squares at the ends of the axes and in the middle are, in pixels of the screen.
        static constexpr float knob = 14.0f;

        /// @brief How thick the lines are and how far the mouse may miss them, in pixels of the screen.
        static constexpr float thickness = 3.0f;
        static constexpr float tolerance = 8.0f;

        /// @brief How many straight pieces make up the ring.
        static constexpr int ringSegments = 48;

        /// @brief What snapping rounds angles and scales to. Positions snap to the grid.
        static constexpr float angleStep = 15.0f;
        static constexpr float scaleStep = 0.25f;

        /// @brief Returns what the gizmo does.
        GizmoMode mode() const;

        /// @brief Chooses what the gizmo does from now on.
        void setMode(GizmoMode mode);

        /// @brief Returns the part of the gizmo around the entity with the transform that lies under the point, if any.
        /// The square in the middle comes before the axes.
        std::optional<GizmoHandle> handleAt(const Transform2D& transform, Vec2 point, float zoom) const;

        /// @brief Takes the part at the point, so that drag measures from there and from the transform as it is now.
        void grab(GizmoHandle handle, const Transform2D& transform, Vec2 point, float zoom);

        /// @brief Returns the part that the gizmo holds, if any.
        std::optional<GizmoHandle> grabbed() const;

        /// @brief Returns the transform from the grab with the mouse at the point. Snapping puts a moved position on
        /// the grid, an angle on a multiple of angleStep and a stretched scale on a multiple of scaleStep, each only
        /// where the part changes it. The gizmo has to hold a part.
        Transform2D drag(Vec2 point, bool snap) const;

        /// @brief Lets go of the part that the gizmo holds.
        void release();

        /// @brief Returns the gizmo around the entity with the transform as sprites of the white texture in world
        /// coordinates, with the lit part in a color of its own: the ring of the rotation, or the two axes with their
        /// ends, diamonds for moving and squares for stretching, and the square in the middle on top.
        std::vector<Sprite> sprites(const Transform2D& transform, float zoom, std::optional<GizmoHandle> lit) const;

    private:
        GizmoMode current = GizmoMode::Move;
        std::optional<GizmoHandle> held;
        Transform2D start;
        Vec2 pressPoint;
        float pressZoom = 1.0f;
    };
}
