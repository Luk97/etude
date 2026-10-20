#pragma once

#include <etude/scene/component_registry.h>
#include <etude/scene/entity.h>
#include <etude/scene/world.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>

#include <optional>

namespace etude::editor {

    /// @brief Shows the pane Inspector with the components of the selected entity, each under a bar with its name and a
    /// button that removes it, and below them a button for each type that the entity lacks, which adds it with its
    /// default values. The widgets change the entity right away, so a gesture around the pane turns their work into
    /// commands. Without a selection, the pane says so. The modifier decides the size and the background of the pane.
    void inspectorPane(
        ui::Context& ui,
        const ui::Modifier& modifier,
        World& world,
        const ComponentRegistry& registry,
        std::optional<Entity> selection
    );
}
