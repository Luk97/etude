#include <etude/ui/context.h>

namespace etude::ui {

    namespace {

        constexpr Color textColor{
            .r = 0.9f,
            .g = 0.9f,
            .b = 0.9f,
        };

        constexpr Color buttonColor{
            .r = 0.25f,
            .g = 0.27f,
            .b = 0.33f,
        };
    }

    void Context::label(std::string_view text) {
        box({
            .label = text,
            .width = SizeRule::fitText(2.0f),
            .height = SizeRule::fitText(3.0f),
            .text = textColor,
        });
    }

    bool Context::button(std::string_view label) {
        const Signal signal = box({
            .label = label,
            .width = SizeRule::fitText(6.0f),
            .height = SizeRule::fitText(3.0f),
            .clickable = true,
            .background = buttonColor,
            .text = textColor,
        });
        return signal.clicked;
    }
}
