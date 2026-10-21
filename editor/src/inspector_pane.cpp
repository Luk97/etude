#include <etude/editor/inspector_pane.h>

#include "inspector/inspector_fields.h"

#include <etude/core/assert.h>
#include <etude/editor/widgets/pane.h>
#include <etude/json/json.h>
#include <etude/scene/component_type.h>
#include <etude/ui/column.h>
#include <etude/ui/row.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>
#include <etude/ui/widgets/button.h>

#include <format>

namespace etude::editor {

    void inspectorPane(
        ui::Context& ui,
        const ui::Modifier& modifier,
        World& world,
        const ComponentRegistry& registry,
        std::optional<Entity> selection
    ) {
        pane(ui, "Inspector", modifier, [&] {
            if (!selection) {
                ui::text(ui, "Nichts ausgewählt");
                return;
            }
            const Entity entity = *selection;

            // Each component lies in a column named by its type, under a bar with a button that removes it and its
            // name. A press on the button removes the component before its fields would show.
            InspectorFields fields(ui);
            for (const ComponentType& type : registry.types()) {
                if (!type.has(world, entity)) {
                    continue;
                }
                const ui::Modifier section = ui::Modifier().key(type.name).fillMaxWidth().spacedBy(ui::theme::spacing);
                ui::column(ui, section, [&] {
                    bool removed = false;
                    const ui::Modifier bar =
                        ui::Modifier().fillMaxWidth().spacedBy(ui::theme::spacing).background(ui::theme::fieldColor);
                    ui::row(ui, bar, [&] {
                        removed = ui::button(ui, "x##remove").clicked;
                        ui::text(ui, type.name);
                    });
                    if (removed) {
                        type.remove(world, entity);
                    } else {
                        type.inspect(world, entity, fields);
                    }
                });
            }

            // Below the components, a button for each type that the entity lacks adds it with its default values, which
            // an empty object reads as.
            for (const ComponentType& type : registry.types()) {
                if (!type.has(world, entity) && ui::button(ui, std::format("+ {}", type.name)).clicked) {
                    [[maybe_unused]] const auto read = type.read(world, entity, Json::Object{});
                    ETUDE_ASSERT(read);
                }
            }
        });
    }
}
