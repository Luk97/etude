#include <etude/ui/modifier.h>

#include <cstddef>

namespace etude::ui {

    namespace {

        /// @brief The indices of the axes in the lengths of the settings.
        constexpr std::size_t x = 0;
        constexpr std::size_t y = 1;
    }

    Modifier Modifier::key(std::string_view name) const {
        Modifier copy = *this;
        copy.settings.key = name;
        return copy;
    }

    Modifier Modifier::width(float pixels) const {
        Modifier copy = *this;
        copy.settings.pixels[x] = pixels;
        return copy;
    }

    Modifier Modifier::height(float pixels) const {
        Modifier copy = *this;
        copy.settings.pixels[y] = pixels;
        return copy;
    }

    Modifier Modifier::size(float width, float height) const {
        Modifier copy = *this;
        copy.settings.pixels = {width, height};
        return copy;
    }

    Modifier Modifier::fillMaxWidth(float share) const {
        Modifier copy = *this;
        copy.settings.shares[x] = share;
        return copy;
    }

    Modifier Modifier::fillMaxHeight(float share) const {
        Modifier copy = *this;
        copy.settings.shares[y] = share;
        return copy;
    }

    Modifier Modifier::fillMaxSize(float share) const {
        Modifier copy = *this;
        copy.settings.shares = {share, share};
        return copy;
    }

    Modifier Modifier::padding(float pixels) const {
        Modifier copy = *this;
        copy.settings.padding = {pixels, pixels};
        return copy;
    }

    Modifier Modifier::padding(float horizontal, float vertical) const {
        Modifier copy = *this;
        copy.settings.padding = {horizontal, vertical};
        return copy;
    }

    Modifier Modifier::spacedBy(float pixels) const {
        Modifier copy = *this;
        copy.settings.spacing = pixels;
        return copy;
    }

    Modifier Modifier::contentAlignment(Alignment alignment) const {
        Modifier copy = *this;
        copy.settings.alignment = alignment;
        return copy;
    }

    Modifier Modifier::background(Color color) const {
        Modifier copy = *this;
        copy.settings.background = color;
        return copy;
    }

    Modifier Modifier::clip() const {
        Modifier copy = *this;
        copy.settings.clip = true;
        return copy;
    }

    Modifier Modifier::clickable() const {
        Modifier copy = *this;
        copy.settings.clickable = true;
        return copy;
    }

    Modifier Modifier::focusable() const {
        Modifier copy = *this;
        copy.settings.focusable = true;
        return copy;
    }

    Modifier Modifier::verticalScroll() const {
        Modifier copy = *this;
        copy.settings.verticalScroll = true;
        return copy;
    }

    Modifier Modifier::floating(Vec2 start) const {
        Modifier copy = *this;
        copy.settings.floating = start;
        return copy;
    }

    Modifier Modifier::dragHandle() const {
        Modifier copy = *this;
        copy.settings.dragHandle = true;
        return copy;
    }
}
