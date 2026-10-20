#pragma once

#include <etude/math/vec2.h>
#include <etude/scene/field_visitor.h>
#include <etude/ui/context.h>

#include <string>
#include <string_view>

namespace etude::editor {

    /// @brief Shows the fields of a component in the inspector, one row for each, with a widget that suits its type:
    /// numbers as number fields, a Vec2 as two of them and strings as text fields. The widgets change the fields right
    /// away.
    class InspectorFields final : public FieldVisitor {
    public:
        explicit InspectorFields(ui::Context& context);

        void visit(std::string_view name, float& value) override;
        void visit(std::string_view name, int& value) override;
        void visit(std::string_view name, std::string& value) override;
        void visit(std::string_view name, Vec2& value) override;

    private:
        ui::Context& ui;
    };
}
