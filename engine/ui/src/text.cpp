#include <etude/ui/text.h>

#include "arrangement.h"

#include <etude/ui/context.h>

namespace etude::ui {

    Interaction text(Context& ui, std::string_view line, const Modifier& modifier) {
        // A text has no children, so it closes right away.
        const Interaction interaction = ui.openElement(modifier, Arrangement::Stack, line);
        ui.closeElement();
        return interaction;
    }
}
