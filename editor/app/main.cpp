#include <etude/core/log.h>
#include <etude/editor/console.h>
#include <etude/editor/console_pane.h>
#include <etude/editor/hierarchy_pane.h>
#include <etude/editor/pane.h>
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
    constexpr float inspectorWidth = 150.0f;
    constexpr float bottomHeight = 90.0f;

    /// @brief The space between the panes, through which the clear color shows as a thin line.
    constexpr float paneSpacing = 1.0f;

    /// @brief The editor: a fixed layout of a toolbar and five panes around the scene, which it shows without running
    /// the simulation.
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
        void onUi(etude::ui::Context& ui) override {
            using etude::ui::Modifier;
            namespace theme = etude::ui::theme;
            const Modifier opaque = Modifier().background(theme::panelColor);
            etude::ui::Interaction view;

            column(ui, Modifier().fillMaxSize().spacedBy(paneSpacing), [&] {
                // The toolbar is a line high and names the scene.
                const Modifier toolbar =
                    Modifier().fillMaxWidth().height(theme::lineHeight).background(theme::buttonColor);
                row(ui, toolbar, [&] { text(ui, startScene.generic_string()); });

                // The middle takes the height that the toolbar and the bottom leave, the scene the width between the
                // hierarchy and the inspector.
                row(ui, Modifier().fillMaxWidth().weight().spacedBy(paneSpacing), [&] {
                    etude::editor::hierarchyPane(ui, opaque.width(hierarchyWidth).fillMaxHeight(), world(), selection);
                    etude::editor::pane(ui, "Szene", Modifier().weight().fillMaxHeight(), [&] {
                        view = box(ui, Modifier().key("View").fillMaxSize());
                    });
                    etude::editor::pane(ui, "Inspector", opaque.width(inspectorWidth).fillMaxHeight(), {});
                });

                // The bottom keeps its height, the assets take half of its width and the console the rest.
                row(ui, Modifier().fillMaxWidth().height(bottomHeight).spacedBy(paneSpacing), [&] {
                    etude::editor::pane(ui, "Assets", opaque.fillMaxWidth(0.5f).fillMaxHeight(), {});
                    etude::editor::consolePane(ui, opaque.weight().fillMaxHeight(), console);
                });
            });

            // The world shows where the view lay in the last frame, since the layout of this frame follows later.
            setViewport(view.rect);
        }

    private:
        etude::editor::Console console;
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
