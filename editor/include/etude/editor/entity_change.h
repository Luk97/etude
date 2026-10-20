#pragma once

#include <etude/json/json.h>
#include <etude/scene/entity.h>

#include <optional>

namespace etude::editor {

    /// @brief What one action did to one entity: its components before and after, as writeEntity writes them. A missing
    /// state means that the entity did not exist, so a change from nothing creates it and a change to nothing destroys
    /// it.
    struct EntityChange {
        Entity entity{};
        std::optional<Json::Object> before;
        std::optional<Json::Object> after;
    };
}
