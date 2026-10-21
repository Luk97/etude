#include <etude/editor/scene/gizmo.h>

#include <etude/editor/scene/grid.h>
#include <etude/math/angle.h>
#include <etude/math/color.h>

#include <cmath>
#include <numbers>

namespace etude::editor {

    namespace {

        /// @brief The colors of the parts: red for x, green for y, blue for the middle and the ring, as in most
        /// editors, and yellow for the part under the mouse or in its hold.
        constexpr Color xColor{
            .r = 0.9f,
            .g = 0.3f,
            .b = 0.3f,
        };
        constexpr Color yColor{
            .r = 0.35f,
            .g = 0.8f,
            .b = 0.35f,
        };
        constexpr Color middleColor{
            .r = 0.35f,
            .g = 0.55f,
            .b = 0.9f,
        };
        constexpr Color litColor{
            .r = 1.0f,
            .g = 0.85f,
            .b = 0.3f,
        };

        /// @brief Returns a line between two points as a sprite of the white texture, as thick as given.
        Sprite line(Vec2 from, Vec2 to, float thickness, Color color) {
            const Vec2 along = to - from;
            const Vec2 size{along.length(), thickness};
            return {
                .position = from + along * 0.5f - size * 0.5f,
                .size = size,
                .rotation = along.angle(),
                .color = color,
            };
        }

        /// @brief Returns a square around the point as a sprite of the white texture, turned by the angle in radians.
        Sprite square(Vec2 middle, float size, float radians, Color color) {
            return {
                .position = middle - Vec2{size, size} * 0.5f,
                .size = {size, size},
                .rotation = radians,
                .color = color,
            };
        }

        /// @brief Rounds the value to the nearest multiple of the step.
        float snapped(float value, float step) {
            return std::round(value / step) * step;
        }

        /// @brief Returns the angle of the axes of the gizmo in radians: those of the entity for stretching, which
        /// stretches before it turns, and those of the world otherwise.
        float axisAngle(GizmoMode mode, const Transform2D& transform) {
            return mode == GizmoMode::Scale ? toRadians(transform.rotation) : 0.0f;
        }
    }

    GizmoMode Gizmo::mode() const {
        return current;
    }

    void Gizmo::setMode(GizmoMode mode) {
        current = mode;
    }

    std::optional<GizmoHandle> Gizmo::handleAt(const Transform2D& transform, Vec2 point, float zoom) const {
        // The point counts in pixels of the screen from the middle of the entity, where the ring lies at a distance.
        const Vec2 offset = (point - transform.position) * zoom;
        if (current == GizmoMode::Rotate) {
            if (std::abs(offset.length() - reach) <= tolerance) {
                return GizmoHandle::Ring;
            }
            return std::nullopt;
        }

        // Along the axes of the gizmo, the square in the middle comes first, then each axis up to the end of its knob.
        const Vec2 local = offset.rotated(-axisAngle(current, transform));
        const float half = knob / 2.0f;
        if (std::abs(local.x) <= half && std::abs(local.y) <= half) {
            return GizmoHandle::Center;
        }
        if (local.x >= 0.0f && local.x <= reach + half && std::abs(local.y) <= tolerance) {
            return GizmoHandle::AxisX;
        }
        if (local.y >= 0.0f && local.y <= reach + half && std::abs(local.x) <= tolerance) {
            return GizmoHandle::AxisY;
        }
        return std::nullopt;
    }

    void Gizmo::grab(GizmoHandle handle, const Transform2D& transform, Vec2 point, float zoom) {
        held = handle;
        start = transform;
        pressPoint = point;
        pressZoom = zoom;
    }

    std::optional<GizmoHandle> Gizmo::grabbed() const {
        return held;
    }

    Transform2D Gizmo::drag(Vec2 point, bool snap) const {
        Transform2D result = start;
        const bool alongX = held != GizmoHandle::AxisY;
        const bool alongY = held != GizmoHandle::AxisX;
        switch (current) {
            case GizmoMode::Move: {
                // The position follows the mouse along the axes that the part moves.
                const Vec2 moved = point - pressPoint;
                if (alongX) {
                    result.position.x = start.position.x + moved.x;
                    result.position.x = snap ? snapped(result.position.x, gridSpacing) : result.position.x;
                }
                if (alongY) {
                    result.position.y = start.position.y + moved.y;
                    result.position.y = snap ? snapped(result.position.y, gridSpacing) : result.position.y;
                }
                break;
            }
            case GizmoMode::Rotate: {
                // The angle grows by how far the mouse went around the middle since the grab, counted between -180 and
                // 180 degrees, so that it does not jump by a whole turn where the angle of a vector does.
                const Vec2 from = pressPoint - start.position;
                const Vec2 to = point - start.position;
                const float turn = toDegrees(to.angle() - from.angle());
                result.rotation = start.rotation + std::remainder(turn, 360.0f);
                result.rotation = snap ? snapped(result.rotation, angleStep) : result.rotation;
                break;
            }
            case GizmoMode::Scale: {
                // An axis stretches by how far the mouse lies from the middle along the axis of the entity, compared
                // with the grab. The square stretches evenly by how far the mouse went to the right, a whole reach on
                // the screen adding the scale once more.
                const float radians = toRadians(start.rotation);
                const Vec2 from = (pressPoint - start.position).rotated(-radians);
                const Vec2 to = (point - start.position).rotated(-radians);
                if (held == GizmoHandle::AxisX) {
                    result.scale.x = start.scale.x * to.x / from.x;
                } else if (held == GizmoHandle::AxisY) {
                    result.scale.y = start.scale.y * to.y / from.y;
                } else {
                    result.scale = start.scale * (1.0f + (point.x - pressPoint.x) * pressZoom / reach);
                }
                if (snap) {
                    result.scale.x = alongX ? snapped(result.scale.x, scaleStep) : result.scale.x;
                    result.scale.y = alongY ? snapped(result.scale.y, scaleStep) : result.scale.y;
                }
                break;
            }
        }
        return result;
    }

    void Gizmo::release() {
        held.reset();
    }

    std::vector<Sprite> Gizmo::sprites(const Transform2D& transform, float zoom, std::optional<GizmoHandle> lit) const {
        // Lengths on the screen become units of the world at the zoom, and the lit part takes its own color.
        const float unit = 1.0f / zoom;
        const Vec2 middle = transform.position;
        const auto colorOf = [&](GizmoHandle handle, Color color) { return handle == lit ? litColor : color; };
        std::vector<Sprite> parts;

        // The ring is a closed line of short straight pieces around the middle.
        if (current == GizmoMode::Rotate) {
            const Vec2 radius{reach * unit, 0.0f};
            const float step = 2.0f * std::numbers::pi_v<float> / ringSegments;
            for (int i = 0; i < ringSegments; ++i) {
                const Vec2 from = middle + radius.rotated(step * i);
                const Vec2 to = middle + radius.rotated(step * (i + 1));
                parts.push_back(line(from, to, thickness * unit, colorOf(GizmoHandle::Ring, middleColor)));
            }
            return parts;
        }

        // Each axis is a line with a knob at its end, a diamond for moving and a square for stretching, and the square
        // in the middle lies on top.
        const float radians = axisAngle(current, transform);
        const float knobAngle = current == GizmoMode::Move ? radians + std::numbers::pi_v<float> / 4.0f : radians;
        const auto axis = [&](Vec2 direction, GizmoHandle handle, Color color) {
            const Vec2 end = middle + direction.rotated(radians) * (reach * unit);
            parts.push_back(line(middle, end, thickness * unit, colorOf(handle, color)));
            parts.push_back(square(end, knob * unit, knobAngle, colorOf(handle, color)));
        };
        axis({1.0f, 0.0f}, GizmoHandle::AxisX, xColor);
        axis({0.0f, 1.0f}, GizmoHandle::AxisY, yColor);
        parts.push_back(square(middle, knob * unit, radians, colorOf(GizmoHandle::Center, middleColor)));
        return parts;
    }
}
