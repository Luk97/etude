#include <etude/editor/scene/scene_pane.h>

#include <etude/editor/scene/grid.h>
#include <etude/editor/scene/outline.h>
#include <etude/editor/widgets/pane.h>
#include <etude/math/color.h>
#include <etude/math/vec2.h>
#include <etude/platform/input.h>
#include <etude/platform/mouse_button.h>
#include <etude/scene/components/camera.h>
#include <etude/scene/components/sprite_renderer.h>
#include <etude/scene/components/transform2d.h>
#include <etude/ui/box.h>
#include <etude/ui/interaction.h>

namespace etude::editor {

    namespace {

        /// @brief The size of the game window, which the frame of a Camera shows, until a project can set it.
        constexpr Vec2 gameSize{1280.0f, 720.0f};

        /// @brief How the pane draws its frames, in pixels of the screen at any zoom: with lines this thick, and around
        /// an entity without a sprite as a square this large.
        constexpr float frameThickness = 2.0f;
        constexpr float markerSize = 12.0f;

        /// @brief The colors of the frame around the selection and of the frames of the cameras.
        constexpr Color selectionColor{
            .r = 1.0f,
            .g = 0.6f,
            .b = 0.15f,
        };
        constexpr Color cameraColor{
            .r = 0.9f,
            .g = 0.9f,
            .b = 0.9f,
        };
    }

    ScenePane::ScenePane(World& sceneWorld, const TextureLookup& textureLookup)
        : world(sceneWorld), textures(textureLookup) {}

    void ScenePane::lookAtSceneCamera() {
        world.each<Camera, Transform2D>([&](Entity, const Camera&, const Transform2D& transform) {
            camera.center = transform.position;
        });
    }

    void ScenePane::show(ui::Context& ui, const ui::Modifier& modifier, std::optional<Entity>& selection) {
        // The world appears in the rectangle that the view had in the last frame, since the layout of this frame
        // follows later.
        ui::Interaction view;
        pane(ui, "Szene", modifier, [&] { view = ui::box(ui, ui::Modifier().key("View").fillMaxSize().clickable()); });
        viewArea = view.rect;
        const Input& input = ui.input();
        const Vec2 mouse = input.mousePosition() - view.rect.position;

        // Dragging starts with the frame after the press, since the mouse may have moved before the button went down,
        // and goes on outside the pane until the button comes up.
        const bool middle = input.held(MouseButton::Middle);
        if (panning && middle) {
            camera.pan(input.mouseMovement());
        }
        panning = middle && (panning || (view.hovered && input.pressed(MouseButton::Middle)));

        // The wheel and the left button only count over the pane.
        if (view.hovered && input.wheel() != 0.0f) {
            camera.zoomAt(input.wheel(), mouse, view.rect.size);
        }
        if (view.pressed) {
            selection = entityAt(world, camera.toWorld(mouse, view.rect.size), textures);
        }
    }

    Rect ScenePane::area() const {
        return viewArea;
    }

    void ScenePane::draw(RenderList& list, std::optional<Entity> selection, Size window) {
        // The camera of the pane takes the place of the one that a Camera of the scene has pointed, and the grid lies
        // behind the world.
        list.camera = camera.view(viewArea.size);
        list.sprites.insert_range(list.sprites.begin(), gridLines(camera, viewArea.size));

        // Each Camera of the scene gets a frame around what it shows of the game window.
        const float thickness = frameThickness / camera.zoom;
        frames.clear();
        world.each<Camera, Transform2D>([&](Entity, const Camera& sceneCamera, const Transform2D& transform) {
            if (sceneCamera.zoom > 0.0f) {
                const Vec2 shown = gameSize / sceneCamera.zoom;
                frames.append_range(outline(transform.position, shown, 0.0f, thickness, cameraColor));
            }
        });

        // The selection gets a frame around its sprite, or a small square on its position without one.
        if (selection && world.has<Transform2D>(*selection)) {
            const Transform2D& transform = world.get<Transform2D>(*selection);
            Vec2 size = Vec2{markerSize, markerSize} / camera.zoom;
            if (const auto* sprite = world.tryGet<SpriteRenderer>(*selection)) {
                if (const std::optional<LoadedTexture> texture = textures(sprite->texture)) {
                    size = spriteSize(*texture, transform);
                }
            }
            frames.append_range(outline(transform.position, size, transform.rotation, thickness, selectionColor));
        }

        // The frames cover the world in a batch of their own, with the camera and the clip of the world.
        list.batches.push_back({
            .viewProjection = list.camera.viewProjection(window, viewArea),
            .clip = viewArea,
            .sprites = frames,
        });
    }
}
