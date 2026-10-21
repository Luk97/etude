#pragma once

#include <etude/editor/scene/editor_camera.h>
#include <etude/editor/scene/gizmo.h>
#include <etude/editor/scene/gizmo_handle.h>
#include <etude/editor/scene/gizmo_mode.h>
#include <etude/math/rect.h>
#include <etude/math/size.h>
#include <etude/render2d/render_list.h>
#include <etude/rendering/sprite.h>
#include <etude/runtime/render_system.h>
#include <etude/scene/components/transform2d.h>
#include <etude/scene/entity.h>
#include <etude/scene/world.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>

#include <optional>
#include <vector>

namespace etude::editor {

    /// @brief The pane Szene, which shows the world through a camera of its own, apart from the Camera of the scene, so
    /// that looking around changes nothing that the scene file keeps. The world lies over a grid, with frames around
    /// the selection and the cameras of the scene, and a gizmo moves, turns and stretches the selection. The middle
    /// mouse button drags the world, the wheel zooms around the mouse, and a press of the left button takes a part of
    /// the gizmo, or else selects the entity on top under the mouse, or nothing. The world and the textures have to
    /// outlive the pane.
    class ScenePane {
    public:
        ScenePane(World& sceneWorld, const TextureLookup& textureLookup);

        /// @brief Points the camera at a Camera of the scene, if it has one, such as after loading the scene.
        void lookAtSceneCamera();

        /// @brief Shows the pane and lets the mouse look around in it, select and work the gizmo, which snaps while
        /// Ctrl is held. The modifier decides the size of the pane and leaves out a background, which would cover the
        /// world.
        void show(ui::Context& ui, const ui::Modifier& modifier, std::optional<Entity>& selection);

        /// @brief Returns what the gizmo does.
        GizmoMode gizmoMode() const;

        /// @brief Chooses what the gizmo does from now on.
        void setGizmoMode(GizmoMode mode);

        /// @brief Returns where the pane showed the world in the last frame, in pixels of the window, which the editor
        /// hands to setViewport.
        Rect area() const;

        /// @brief Points the camera of the list at what the pane shows, puts the grid behind the world and the frames
        /// and the gizmo on top, for onDraw in a window of the given size.
        void draw(RenderList& list, std::optional<Entity> selection, Size window);

    private:
        /// @brief Returns the Transform2D of the entity, if there is an entity with one.
        Transform2D* transformOf(std::optional<Entity> entity);

        World& world;
        const TextureLookup& textures;
        EditorCamera camera;
        Gizmo gizmo;
        bool panning = false;
        Rect viewArea;

        /// @brief The part of the gizmo that lights up in this frame.
        std::optional<GizmoHandle> lit;

        /// @brief The frames that the pane draws over the world, which have to live until the frame is drawn.
        std::vector<Sprite> frames;
    };
}
