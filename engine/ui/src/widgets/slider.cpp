#include <etude/ui/widgets/slider.h>

#include <etude/ui/box.h>
#include <etude/ui/context.h>
#include <etude/ui/label.h>
#include <etude/ui/modifier.h>
#include <etude/ui/row.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>

#include <algorithm>
#include <format>

namespace etude::ui {

    namespace {

        /// @brief The width of the bar, in pixels at scale 1.
        constexpr float trackWidth = 100.0f;
    }

    Interaction slider(Context& ui, std::string_view label, float& value, float min, float max) {
        Interaction interaction;

        // The row takes no clicks, but its key keeps the bars of two sliders apart.
        row(ui, Modifier().key(label), [&] {
            text(ui, visibleText(label));

            // The bar reads the mouse against its rectangle of the last frame and fills up to the value.
            const Modifier track = Modifier()
                                       .key("##track")
                                       .size(trackWidth, theme::lineHeight)
                                       .background(theme::buttonColor)
                                       .clickable();
            interaction = box(ui, track, [&](const Interaction& self) {
                // From the press on and while the bar is held, the value follows the mouse, kept between min and max. A
                // press that comes up again within the same frame, such as a tap on a touchpad, counts as well.
                if (self.pressed || self.held) {
                    const float share = (ui.input().mousePosition().x - self.rect.position.x) / self.rect.size.x;
                    value = min + (max - min) * std::clamp(share, 0.0f, 1.0f);
                }
                const Modifier fill =
                    Modifier().fillMaxWidth((value - min) / (max - min)).fillMaxHeight().background(theme::accentColor);
                box(ui, fill);
            });
            text(ui, std::format("{:.2f}", value));
        });
        return interaction;
    }
}
