#include "inspector_fields.h"

#include <etude/ui/modifier.h>
#include <etude/ui/row.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>
#include <etude/ui/widgets/number_field.h>
#include <etude/ui/widgets/text_field.h>

namespace etude::editor {

    InspectorFields::InspectorFields(ui::Context& context) : ui(context) {}

    void InspectorFields::visit(std::string_view name, float& value) {
        ui::numberField(ui, name, value);
    }

    void InspectorFields::visit(std::string_view name, int& value) {
        ui::numberField(ui, name, value);
    }

    void InspectorFields::visit(std::string_view name, std::string& value) {
        ui::textField(ui, name, value);
    }

    void InspectorFields::visit(std::string_view name, Vec2& value) {
        // The name of the row keeps the parts x and y of two fields apart.
        ui::row(ui, ui::Modifier().key(name).spacedBy(ui::theme::spacing), [&] {
            ui::text(ui, name);
            ui::numberField(ui, "x", value.x);
            ui::numberField(ui, "y", value.y);
        });
    }
}
