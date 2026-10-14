#include <etude/ui/column.h>

#include "arrangement.h"

#include <etude/ui/context.h>

#include <optional>

namespace etude::ui {

    Interaction column(Context& ui, const Modifier& modifier, Content content) {
        // Whatever the content adds while the column is open becomes its children.
        const Interaction interaction = ui.openElement(modifier, Arrangement::Column, std::nullopt);
        content(interaction);
        ui.closeElement();
        return interaction;
    }

    Interaction column(Context& ui, Content content) {
        return column(ui, Modifier(), content);
    }
}
