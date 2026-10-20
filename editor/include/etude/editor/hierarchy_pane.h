#pragma once

#include <etude/scene/entity.h>
#include <etude/scene/world.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>

#include <optional>

namespace etude::editor {

    /// @brief Shows the pane Hierarchie, which lists the entities of the world in the order of their indices, each by
    /// its Name, or as Entity and its index without one. A click on an entity selects it. The modifier decides the size
    /// and the background of the pane.
    void hierarchyPane(
        ui::Context& ui,
        const ui::Modifier& modifier,
        const World& world,
        std::optional<Entity>& selection
    );
}
