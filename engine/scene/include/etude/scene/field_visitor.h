#pragma once

#include <etude/math/vec2.h>
#include <etude/scene/reflection.h>

#include <string>
#include <string_view>
#include <tuple>

namespace etude {

    /// @brief Sees the fields of a component one after another, with their names, and may change them. Code that does
    /// not know the type of the component can work with its fields this way, such as the inspector of the editor, which
    /// shows a widget for each field.
    ///
    /// There is one function for each type that fields can have, the same types that toJson and fromJson know.
    class FieldVisitor {
    public:
        virtual ~FieldVisitor() = default;

        virtual void visit(std::string_view name, float& value) = 0;
        virtual void visit(std::string_view name, int& value) = 0;
        virtual void visit(std::string_view name, std::string& value) = 0;
        virtual void visit(std::string_view name, Vec2& value) = 0;
    };

    /// @brief Hands each field of the component to the visitor, in the order of the field list.
    template <Reflected Component>
    void visitFields(Component& component, FieldVisitor& visitor) {
        std::apply(
            [&](const auto&... field) { (visitor.visit(field.name, component.*field.member), ...); }, Component::fields
        );
    }
}
