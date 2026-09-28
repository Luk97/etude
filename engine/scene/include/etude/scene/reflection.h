#pragma once

#include <concepts>
#include <string_view>
#include <tuple>

namespace etude {

    /// @brief Names a member of a component, so that code can read and write the member without knowing the type.
    template <typename Owner, typename Value>
    struct Field {
        std::string_view name;
        Value Owner::* member;
    };

    /// @brief A component that scenes can hold. It has a fixed type name, which files keep even if the C++ type gets
    /// renamed, and a tuple of fields in the order in which files list them. Reading starts from the default values.
    template <typename Component>
    concept Reflected = std::default_initializable<Component> && requires {
        { Component::typeName } -> std::convertible_to<std::string_view>;
        Component::fields;
    };

    /// @brief Returns whether the component has a field with the name.
    template <Reflected Component>
    constexpr bool hasField(std::string_view name) {
        return std::apply([&](const auto&... field) { return (... || (field.name == name)); }, Component::fields);
    }
}
