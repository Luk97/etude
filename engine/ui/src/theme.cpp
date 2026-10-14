#include <etude/ui/theme.h>

namespace etude::ui::theme {

    namespace {

        constexpr Color white{
            .r = 1.0f,
            .g = 1.0f,
            .b = 1.0f,
        };

        constexpr Color black{};

        /// @brief Mixes two colors, from the first at 0 to the second at 1, and keeps the alpha of the first.
        Color mix(Color from, Color to, float amount) {
            return {
                .r = from.r + (to.r - from.r) * amount,
                .g = from.g + (to.g - from.g) * amount,
                .b = from.b + (to.b - from.b) * amount,
                .a = from.a,
            };
        }
    }

    Color highlight(Color color, bool hovered, bool held) {
        // Holding comes first, since a held element mostly lies under the mouse as well.
        if (held) {
            return mix(color, black, 0.25f);
        }
        if (hovered) {
            return mix(color, white, 0.15f);
        }
        return color;
    }
}
