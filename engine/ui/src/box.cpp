#include <etude/ui/box.h>

#include "arrangement.h"

#include <etude/ui/context.h>

#include <optional>

namespace etude::ui {

    Interaction box(Context& ui, const Modifier& modifier, Content content) {
        // Whatever the content adds while the box is open becomes its children.
        const Interaction interaction = ui.openElement(modifier, Arrangement::Stack, std::nullopt);
        content(interaction);
        ui.closeElement();
        return interaction;
    }

    Interaction box(Context& ui, Content content) {
        return box(ui, Modifier(), content);
    }
}
