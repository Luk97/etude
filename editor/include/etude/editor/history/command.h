#pragma once

#include <etude/editor/history/entity_change.h>

#include <vector>

namespace etude::editor {

    /// @brief One action of the editor, which undo takes back as a whole: the changes of all the entities that it
    /// touched.
    struct Command {
        std::vector<EntityChange> changes;
    };
}
