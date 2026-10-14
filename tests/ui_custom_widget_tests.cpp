#include <catch2/catch_test_macros.hpp>

#include <etude/ui/context.h>
#include <etude/ui/label.h>
#include <etude/ui/modifier.h>
#include <etude/ui/row.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>

#include <algorithm>
#include <string_view>

using etude::Input;
using etude::MouseButton;
using etude::PixelFont;
using etude::ui::Context;
using etude::ui::Interaction;
using etude::ui::Modifier;

namespace {

    constexpr auto fontTexture = static_cast<etude::TextureId>(7);
    constexpr etude::Size screen{640, 360};

    /// @brief A widget as a game would write it, from nothing but the public parts of etude.ui, like the built-in
    /// widgets: a switch that shows its label and An or Aus, and flips the value on a click.
    Interaction toggle(Context& ui, std::string_view label, bool& value) {
        // The whole switch is one clickable row, colored by its value.
        const Modifier frame = Modifier()
                                   .key(label)
                                   .background(value ? etude::ui::theme::accentColor : etude::ui::theme::buttonColor)
                                   .clickable();
        return row(ui, frame, [&](const Interaction& self) {
            // As in the checkbox, the click flips the value before the texts show it.
            if (self.clicked) {
                value = !value;
            }
            text(ui, etude::ui::visibleText(label));
            text(ui, value ? "An" : "Aus");
        });
    }
}

TEST_CASE("A widget of the game works with nothing but the public parts of etude.ui") {
    Input input;
    Context ui{PixelFont{fontTexture}};
    bool light = false;
    const auto frame = [&] {
        ui.beginFrame(input, screen, 1);
        const bool clicked = toggle(ui, "Licht##1", light).clicked;
        ui.endFrame();
        input.beginFrame();
        return clicked;
    };
    const auto glyphs = [&] {
        return std::ranges::count_if(ui.sprites(), [](const etude::Sprite& sprite) {
            return sprite.texture == fontTexture;
        });
    };
    frame();

    // Licht and Aus make eight characters, the part after ## shows nowhere.
    CHECK(glyphs() == 8);

    // The press alone flips nothing, the release does, and An is one character shorter than Aus.
    input.onMouseMove({5.0f, 5.0f});
    input.onMouseButtonDown(MouseButton::Left);
    CHECK_FALSE(frame());
    input.onMouseButtonUp(MouseButton::Left);
    CHECK(frame());
    CHECK(light);
    CHECK(glyphs() == 7);
}
