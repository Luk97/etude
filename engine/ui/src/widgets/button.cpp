#include <etude/ui/widgets/button.h>

#include <etude/ui/box.h>
#include <etude/ui/label.h>
#include <etude/ui/modifier.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>

namespace etude::ui {

    namespace {

        /// @brief The space left and right of the text, in pixels at scale 1.
        constexpr float textPadding = 6.0f;
    }

    Interaction button(Context& ui, std::string_view label) {
        // The whole label names the clickable box, whose background lights up under the mouse by itself. The text keeps
        // the height of a line, but gets more room to the left and the right.
        const Modifier frame = Modifier().key(label).background(theme::buttonColor).clickable();
        return box(ui, frame, [&] {
            text(ui, visibleText(label), Modifier().padding(textPadding, theme::linePadding));
        });
    }
}
