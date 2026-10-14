#include <etude/ui/row.h>

#include "arrangement.h"

#include <etude/ui/context.h>

#include <optional>

namespace etude::ui {

    Interaction row(Context& ui, const Modifier& modifier, Content content) {
        // Whatever the content adds while the row is open becomes its children.
        const Interaction interaction = ui.openElement(modifier, Arrangement::Row, std::nullopt);
        content(interaction);
        ui.closeElement();
        return interaction;
    }

    Interaction row(Context& ui, Content content) {
        return row(ui, Modifier(), content);
    }
}
