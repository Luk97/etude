#include <etude/core/log.h>
#include <etude/editor/command.h>
#include <etude/editor/console.h>
#include <etude/editor/console_pane.h>
#include <etude/editor/editor_camera.h>
#include <etude/editor/entity_commands.h>
#include <etude/editor/gesture.h>
#include <etude/editor/grid.h>
#include <etude/editor/hierarchy_pane.h>
#include <etude/editor/history.h>
#include <etude/editor/inspector_pane.h>
#include <etude/editor/outline.h>
#include <etude/editor/pane.h>
#include <etude/math/color.h>
#include <etude/math/rect.h>
#include <etude/math/vec2.h>
#include <etude/platform/input.h>
#include <etude/platform/key.h>
#include <etude/platform/mouse_button.h>
#include <etude/render2d/render_list.h>
#include <etude/rendering/sprite.h>
#include <etude/runtime/application.h>
#include <etude/runtime/render_system.h>
#include <etude/scene/components/camera.h>
#include <etude/scene/components/sprite_renderer.h>
#include <etude/scene/components/transform2d.h>
#include <etude/scene/entity.h>
#include <etude/scene/scene_file.h>
#include <etude/ui/box.h>
#include <etude/ui/column.h>
#include <etude/ui/context.h>
#include <etude/ui/interaction.h>
#include <etude/ui/modifier.h>
#include <etude/ui/row.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>
#include <etude/ui/widgets/button.h>

#include <filesystem>
#include <format>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace {

    /// @brief The asset folder and the scene that the editor opens, until it can open projects.
    const std::filesystem::path startFolder = EDITOR_START_FOLDER;
    const std::filesystem::path startScene = "scenes/testbed.json";

    /// @brief The fixed lengths of the layout, in pixels at scale 1. The scene pane takes what is left.
    constexpr float hierarchyWidth = 120.0f;
    constexpr float inspectorWidth = 200.0f;
    constexpr float bottomHeight = 90.0f;

    /// @brief The space between the panes, through which the clear color shows as a thin line.
    constexpr float paneSpacing = 1.0f;

    /// @brief The size of the game window, which the frame of a Camera shows, until a project can set it.
    constexpr etude::Vec2 gameSize{1280.0f, 720.0f};

    /// @brief How the scene pane draws its frames, in pixels of the screen at any zoom: with lines this thick, and
    /// around an entity without a sprite as a square this large.
    constexpr float frameThickness = 2.0f;
    constexpr float markerSize = 12.0f;

    /// @brief The colors of the frame around the selection and of the frames of the cameras.
    constexpr etude::Color selectionColor{
        .r = 1.0f,
        .g = 0.6f,
        .b = 0.15f,
    };
    constexpr etude::Color cameraColor{
        .r = 0.9f,
        .g = 0.9f,
        .b = 0.9f,
    };

    /// @brief The editor: a fixed layout of a toolbar and five panes around the scene, which it shows without running
    /// the simulation. Every change of the scene goes into the history, so that undo can take it back. The scene pane
    /// looks at the world through a camera of its own, over a grid and with frames around the selection and the cameras
    /// of the scene.
    class Editor final : public etude::Application {
    public:
        Editor() : Application("ETUDE Editor", 1600, 900, startFolder) {
            // From now on, every message of the log goes into the console as well. The editor shows the scene without
            // running it.
            etude::setLogListener([this](etude::LogLevel level, std::string_view message) {
                console.add(level, message);
            });
            setSimulating(false);
        }

        ~Editor() override {
            etude::setLogListener({});
        }

        /// @brief Replaces the world by the start scene. Returns false after logging why if that fails.
        bool load() {
            auto loaded = etude::loadScene(startFolder / startScene, registry());
            if (!loaded) {
                etude::logError("{}", loaded.error());
                return false;
            }
            world() = std::move(*loaded);
            setTitle(std::format("ETUDE Editor | {}", startScene.generic_string()));
            etude::logInfo("{} entities loaded from {}", world().size(), startScene.generic_string());

            // The editor camera starts over a Camera of the scene, if it has one.
            world().each<etude::Camera, etude::Transform2D>([&](etude::Entity, const etude::Camera&,
                                                                const etude::Transform2D& transform) {
                camera.center = transform.position;
            });
            return true;
        }

    protected:
        void onFrame(const etude::Input& input) override {
            // The keys of the editor only act while no field takes the keyboard and no gesture is open, which undo and
            // redo would get mixed up with.
            if (ui().wantsKeyboard() || gesture.open()) {
                return;
            }
            const bool control = input.held(etude::Key::Control);
            if (control && input.repeated(etude::Key::Z)) {
                perform(input.held(etude::Key::Shift) ? Action::Redo : Action::Undo);
            }
            if (control && input.repeated(etude::Key::Y)) {
                perform(Action::Redo);
            }
            if (input.pressed(etude::Key::Delete)) {
                perform(Action::Destroy);
            }
        }

        void onUi(etude::ui::Context& ui) override {
            using etude::ui::Modifier;
            namespace theme = etude::ui::theme;
            const Modifier opaque = Modifier().background(theme::panelColor);
            etude::ui::Interaction view;
            std::optional<Action> requested;

            column(ui, Modifier().fillMaxSize().spacedBy(paneSpacing), [&] {
                // The toolbar is a line high, with the buttons of the editor and the name of the scene. Its actions
                // wait until the inspector has settled its gesture.
                const Modifier toolbar = Modifier()
                                             .fillMaxWidth()
                                             .height(theme::lineHeight)
                                             .spacedBy(theme::spacing)
                                             .background(theme::buttonColor);
                row(ui, toolbar, [&] {
                    if (button(ui, "Neu").clicked) {
                        requested = Action::Create;
                    }
                    if (button(ui, "Löschen").clicked) {
                        requested = Action::Destroy;
                    }
                    if (button(ui, "Rückgängig").clicked) {
                        requested = Action::Undo;
                    }
                    if (button(ui, "Wiederholen").clicked) {
                        requested = Action::Redo;
                    }
                    text(ui, startScene.generic_string());
                });

                // The middle takes the height that the toolbar and the bottom leave, the scene the width between the
                // hierarchy and the inspector.
                row(ui, Modifier().fillMaxWidth().weight().spacedBy(paneSpacing), [&] {
                    etude::editor::hierarchyPane(ui, opaque.width(hierarchyWidth).fillMaxHeight(), world(), selection);
                    etude::editor::pane(ui, "Szene", Modifier().weight().fillMaxHeight(), [&] {
                        view = box(ui, Modifier().key("View").fillMaxSize().clickable());
                    });
                    handleSceneMouse(view, ui.input());

                    // Whatever the inspector does to the selected entity in this frame becomes part of the gesture,
                    // which turns into a command once neither a mouse button nor the focus holds it.
                    gesture.watch(world(), registry(), selection);
                    const Modifier inspector = opaque.width(inspectorWidth).fillMaxHeight();
                    etude::editor::inspectorPane(ui, inspector, world(), registry(), selection);
                    const bool ongoing = ui.input().held(etude::MouseButton::Left) || ui.wantsKeyboard();
                    if (std::optional<etude::editor::Command> command = gesture.settle(world(), registry(), ongoing)) {
                        history.push(std::move(*command));
                    }
                });

                // The bottom keeps its height, the assets take half of its width and the console the rest.
                row(ui, Modifier().fillMaxWidth().height(bottomHeight).spacedBy(paneSpacing), [&] {
                    etude::editor::pane(ui, "Assets", opaque.fillMaxWidth(0.5f).fillMaxHeight(), {});
                    etude::editor::consolePane(ui, opaque.weight().fillMaxHeight(), console);
                });
            });

            // The world shows where the view lay in the last frame, since the layout of this frame follows later.
            sceneArea = view.rect;
            setViewport(sceneArea);
            if (requested) {
                perform(*requested);
            }
        }

        void onDraw(etude::RenderList& list) override {
            // The editor camera takes the place of the one that a Camera of the scene has pointed, and the grid lies
            // behind the world.
            list.camera = camera.view(sceneArea.size);
            list.sprites.insert_range(list.sprites.begin(), etude::editor::gridLines(camera, sceneArea.size));

            // Each Camera of the scene gets a frame around what it shows of the game window.
            const float thickness = frameThickness / camera.zoom;
            frames.clear();
            world().each<etude::Camera, etude::Transform2D>([&](etude::Entity, const etude::Camera& sceneCamera,
                                                                const etude::Transform2D& transform) {
                if (sceneCamera.zoom > 0.0f) {
                    const etude::Vec2 shown = gameSize / sceneCamera.zoom;
                    frames.append_range(
                        etude::editor::outline(transform.position, shown, 0.0f, thickness, cameraColor)
                    );
                }
            });

            // The selection gets a frame around its sprite, or a small square on its position without one.
            if (selection && world().has<etude::Transform2D>(*selection)) {
                const etude::Transform2D& transform = world().get<etude::Transform2D>(*selection);
                etude::Vec2 size = etude::Vec2{markerSize, markerSize} / camera.zoom;
                if (const auto* sprite = world().tryGet<etude::SpriteRenderer>(*selection)) {
                    if (const std::optional<etude::LoadedTexture> texture = textures()(sprite->texture)) {
                        size = etude::spriteSize(*texture, transform);
                    }
                }
                frames.append_range(
                    etude::editor::outline(transform.position, size, transform.rotation, thickness, selectionColor)
                );
            }

            // The frames cover the world in a batch of their own, with the camera and the clip of the world.
            list.batches.push_back({
                .viewProjection = list.camera.viewProjection(windowSize(), sceneArea),
                .clip = sceneArea,
                .sprites = frames,
            });
        }

    private:
        /// @brief What the toolbar and the keys of the editor ask for.
        enum class Action {
            Create,
            Destroy,
            Undo,
            Redo
        };

        /// @brief Does what the toolbar or a key asked for. The selection forgets an entity that no longer lives.
        void perform(Action action) {
            switch (action) {
                case Action::Create:
                    selection = etude::editor::createEntity(world(), registry(), history, {});
                    break;
                case Action::Destroy:
                    if (selection) {
                        etude::editor::destroyEntity(world(), registry(), history, *selection);
                    }
                    break;
                case Action::Undo:
                    history.undo(world(), registry());
                    break;
                case Action::Redo:
                    history.redo(world(), registry());
                    break;
            }
            if (selection && !world().alive(*selection)) {
                selection.reset();
            }
        }

        /// @brief Lets the mouse look around in the scene and select in it: the middle button drags the world, the
        /// wheel zooms around the mouse, and a press of the left button selects the entity on top under the mouse, or
        /// nothing.
        void handleSceneMouse(const etude::ui::Interaction& view, const etude::Input& input) {
            const etude::Vec2 mouse = input.mousePosition() - view.rect.position;

            // Dragging starts with the frame after the press, since the mouse may have moved before the button went
            // down, and goes on outside the pane until the button comes up.
            const bool middle = input.held(etude::MouseButton::Middle);
            if (panning && middle) {
                camera.pan(input.mouseMovement());
            }
            panning = middle && (panning || (view.hovered && input.pressed(etude::MouseButton::Middle)));

            // The wheel and the left button only count over the pane.
            if (view.hovered && input.wheel() != 0.0f) {
                camera.zoomAt(input.wheel(), mouse, view.rect.size);
            }
            if (view.pressed) {
                selection = etude::entityAt(world(), camera.toWorld(mouse, view.rect.size), textures());
            }
        }

        etude::editor::Console console;
        etude::editor::History history;
        etude::editor::Gesture gesture;
        std::optional<etude::Entity> selection;
        etude::editor::EditorCamera camera;
        bool panning = false;

        /// @brief Where the scene pane showed the world in the last frame, in pixels of the window.
        etude::Rect sceneArea;

        /// @brief The frames that the scene pane draws over the world, which have to live until the frame is drawn.
        std::vector<etude::Sprite> frames;
    };
}

int main() {
    Editor editor;
    if (!editor.load()) {
        return 1;
    }
    editor.run();
    return 0;
}
