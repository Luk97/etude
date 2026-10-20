#include <etude/ui/widgets/number_field.h>

#include <etude/platform/input.h>
#include <etude/ui/context.h>
#include <etude/ui/label.h>
#include <etude/ui/modifier.h>
#include <etude/ui/row.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>
#include <etude/ui/widgets/text_editing.h>

#include <charconv>
#include <cmath>
#include <format>
#include <optional>
#include <string>
#include <system_error>

namespace etude::ui {

    namespace {

        /// @brief The width of the number, in pixels at scale 1.
        constexpr float fieldWidth = 48.0f;

        /// @brief How much finer dragging gets while Shift is held.
        constexpr float fineDivisor = 100.0f;

        /// @brief Returns the number as the field shows it: a float with up to six significant digits, so that dragging
        /// in small steps does not show the rounding of floats.
        std::string shown(float value) {
            return std::format("{:.6g}", value);
        }

        std::string shown(int value) {
            return std::format("{}", value);
        }

        /// @brief Changes the value by the step for each pixel that the mouse moved in this frame.
        void drag(const Input& input, float& value, float step) {
            const float perPixel = input.held(Key::Shift) ? step / fineDivisor : step;
            value += input.mouseMovement().x * perPixel;
        }

        void drag(const Input& input, int& value, float step) {
            value += static_cast<int>(std::lround(input.mouseMovement().x * step));
        }

        /// @brief Takes the text as the new value if all of it reads as a number of the type.
        template <typename Number>
        void read(std::string_view text, Number& value) {
            Number parsed{};
            const char* end = text.data() + text.size();
            if (const auto [stop, error] = std::from_chars(text.data(), end, parsed);
                error == std::errc{} && stop == end) {
                value = parsed;
            }
        }

        /// @brief Does the work of both number fields, which only differ in how they drag, show and read.
        template <typename Number>
        Interaction numberRow(Context& ui, std::string_view label, Number& value, float step) {
            Interaction interaction;

            // The row takes no clicks, but its key keeps the parts of two number fields apart.
            row(ui, Modifier().key(label), [&] {
                // Holding the label drags the value with the mouse. Where the mouse went in the frame of the press, it
                // went before the button came down, so dragging starts with the next frame.
                const Interaction handle = text(ui, visibleText(label), Modifier().key("##drag").clickable());
                if (handle.held && !handle.pressed) {
                    drag(ui.input(), value, step);
                }

                // The field takes the focus with a press. While it has it, the keys of the frame edit the text that the
                // context keeps for it, which a press fills with the value, and the value follows the text whenever the
                // keys change it.
                const Modifier field = Modifier()
                                           .key("##number")
                                           .width(fieldWidth)
                                           .padding(theme::linePadding)
                                           .background(theme::fieldColor)
                                           .clip()
                                           .focusable();
                interaction = row(ui, field, [&](const Interaction& self) {
                    if (!self.focused) {
                        textWithCursor(ui, shown(value), std::nullopt);
                        return;
                    }
                    std::string& typed = ui.focusText();
                    if (self.pressed) {
                        typed = shown(value);
                    }
                    if (editText(ui.input(), typed, ui.textCursor())) {
                        read(typed, value);
                    }
                    textWithCursor(ui, typed, ui.textCursor());
                });
            });
            return interaction;
        }
    }

    Interaction numberField(Context& ui, std::string_view label, float& value, float step) {
        return numberRow(ui, label, value, step);
    }

    Interaction numberField(Context& ui, std::string_view label, int& value) {
        return numberRow(ui, label, value, 1.0f);
    }
}
