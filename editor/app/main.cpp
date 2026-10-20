#include <etude/core/log.h>
#include <etude/editor/command.h>
#include <etude/editor/console.h>
#include <etude/editor/console_pane.h>
#include <etude/editor/entity_commands.h>
#include <etude/editor/gesture.h>
#include <etude/editor/hierarchy_pane.h>
#include <etude/editor/history.h>
#include <etude/editor/inspector_pane.h>
#include <etude/editor/pane.h>
#include <etude/platform/input.h>
#include <etude/platform/key.h>
#include <etude/platform/mouse_button.h>
#include <etude/runtime/application.h>
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

    /// @brief The editor: a fixed layout of a toolbar and five panes around the scene, which it shows without running
    /// the simulation. Every change of the scene goes into the history, so that undo can take it back.
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
                        view = box(ui, Modifier().key("View").fillMaxSize());
                    });

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
            setViewport(view.rect);
            if (requested) {
                perform(*requested);
            }
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

        etude::editor::Console console;
        etude::editor::History history;
        etude::editor::Gesture gesture;
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
