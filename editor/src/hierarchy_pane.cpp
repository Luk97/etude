#include <etude/editor/hierarchy_pane.h>

#include <etude/editor/list_item.h>
#include <etude/editor/pane.h>
#include <etude/scene/components/name.h>

#include <cstdint>
#include <format>
#include <string>

namespace etude::editor {

    void hierarchyPane(
        ui::Context& ui,
        const ui::Modifier& modifier,
        const World& world,
        std::optional<Entity>& selection
    ) {
        pane(ui, "Hierarchie", modifier, [&] {
            for (const Entity entity : world.entities()) {
                // Names may repeat or be empty, so the index after ## keeps the lines apart.
                const std::uint32_t index = indexOf(entity);
                const Name* name = world.tryGet<Name>(entity);
                const std::string shown =
                    name != nullptr && !name->value.empty() ? name->value : std::format("Entity {}", index);
                if (listItem(ui, std::format("{}##{}", shown, index), selection == entity).clicked) {
                    selection = entity;
                }
            }
        });
    }
}
