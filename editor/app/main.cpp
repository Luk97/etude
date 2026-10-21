#include <etude/core/log.h>
#include <etude/editor/console/console_pane.h>
#include <etude/editor/hierarchy_pane.h>
#include <etude/editor/history/edit_history.h>
#include <etude/editor/inspector_pane.h>
#include <etude/editor/scene/gizmo_mode.h>
#include <etude/editor/scene/scene_pane.h>
#include <etude/editor/widgets/pane.h>
#include <etude/platform/input.h>
#include <etude/platform/key.h>
#include <etude/platform/mouse_button.h>
#include <etude/render2d/render_list.h>
#include <etude/runtime/application.h>
#include <etude/scene/entity.h>
#include <etude/scene/scene_file.h>
#include <etude/ui/column.h>
#include <etude/ui/context.h>
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

    /// @brief Returns what the toolbar shows for a mode of the gizmo, with the key that chooses it.
    std::string_view modeLabel(etude::editor::GizmoMode mode) {
        switch (mode) {
            case etude::editor::GizmoMode::Move:
                return "Verschieben (W)";
            case etude::editor::GizmoMode::Rotate:
                return "Drehen (E)";
            case etude::editor::GizmoMode::Scale:
                return "Skalieren (R)";
        }
        std::unreachable();
    }

    /// @brief The editor: a fixed layout of a toolbar and five panes around the scene, which it shows without running
    /// the simulation. Every change of the scene goes into the history, so that undo can take it back.
    class Editor final : public etude::Application {
    public:
        Editor()
            : Application("ETUDE Editor", 1600, 900, startFolder), edits(world(), registry()),
              scene(world(), textures()) {
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
            scene.lookAtSceneCamera();
            return true;
        }

    protected:
        void onFrame(const etude::Input& input) override {
            // The keys of the editor only act while no field takes the keyboard and no edit is open, which undo and
            // redo would get mixed up with.
            if (ui().wantsKeyboard() || edits.busy()) {
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

            // W, E and R choose what the gizmo does, as in most editors.
            if (input.pressed(etude::Key::W)) {
                scene.setGizmoMode(etude::editor::GizmoMode::Move);
            }
            if (input.pressed(etude::Key::E)) {
                scene.setGizmoMode(etude::editor::GizmoMode::Rotate);
            }
            if (input.pressed(etude::Key::R)) {
                scene.setGizmoMode(etude::editor::GizmoMode::Scale);
            }
        }

        void onUi(etude::ui::Context& ui) override {
            using etude::ui::Modifier;
            namespace theme = etude::ui::theme;
            const Modifier opaque = Modifier().background(theme::panelColor);
            std::optional<Action> requested;

            column(ui, Modifier().fillMaxSize().spacedBy(paneSpacing), [&] {
                // The toolbar is a line high, with the buttons of the editor, the mode of the gizmo and the name of the
                // scene. Its actions wait until the history has settled the edit of this frame.
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
                    text(ui, modeLabel(scene.gizmoMode()));
                    text(ui, startScene.generic_string());
                });

                // The middle takes the height that the toolbar and the bottom leave, the scene the width between the
                // hierarchy and the inspector. Whatever the scene pane and the inspector do to the selected entity in
                // this frame becomes part of the edit, which turns into a command once neither the mouse button nor the
                // focus holds it.
                row(ui, Modifier().fillMaxWidth().weight().spacedBy(paneSpacing), [&] {
                    etude::editor::hierarchyPane(ui, opaque.width(hierarchyWidth).fillMaxHeight(), world(), selection);
                    edits.watch(selection);
                    scene.show(ui, Modifier().weight().fillMaxHeight(), selection);
                    const Modifier inspector = opaque.width(inspectorWidth).fillMaxHeight();
                    etude::editor::inspectorPane(ui, inspector, world(), registry(), selection);
                    edits.settle(ui.input().held(etude::MouseButton::Left) || ui.wantsKeyboard());
                });

                // The bottom keeps its height, the assets take half of its width and the console the rest.
                row(ui, Modifier().fillMaxWidth().height(bottomHeight).spacedBy(paneSpacing), [&] {
                    etude::editor::pane(ui, "Assets", opaque.fillMaxWidth(0.5f).fillMaxHeight(), {});
                    console.show(ui, opaque.weight().fillMaxHeight());
                });
            });

            // The world shows where the scene pane lay in the last frame.
            setViewport(scene.area());
            if (requested) {
                perform(*requested);
            }
        }

        void onDraw(etude::RenderList& list) override {
            scene.draw(list, selection, windowSize());
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
                    selection = edits.create({});
                    break;
                case Action::Destroy:
                    if (selection) {
                        edits.destroy(*selection);
                    }
                    break;
                case Action::Undo:
                    edits.undo();
                    break;
                case Action::Redo:
                    edits.redo();
                    break;
            }
            if (selection && !world().alive(*selection)) {
                selection.reset();
            }
        }

        etude::editor::ConsolePane console;
        etude::editor::EditHistory edits;
        etude::editor::ScenePane scene;
        std::optional<etude::Entity> selection;
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
