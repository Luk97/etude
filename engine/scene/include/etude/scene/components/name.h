#pragma once

#include <etude/scene/reflection.h>

#include <string>
#include <string_view>
#include <tuple>

namespace etude {

    /// @brief Names an entity for the editor and for scripts. Several entities may have the same name.
    struct Name {
        std::string value;

        static constexpr std::string_view typeName = "Name";
        static constexpr std::tuple fields{
            Field{"value", &Name::value},
        };
    };
}
