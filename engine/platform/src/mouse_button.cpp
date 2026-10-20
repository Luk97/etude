#include <etude/platform/mouse_button.h>

#include <array>

namespace etude {

    namespace {

        /// @brief The name of each mouse button, in the order of MouseButton.
        constexpr auto mouseButtonNames = std::to_array<std::string_view>({"Left", "Right", "Middle"});
        static_assert(mouseButtonNames.size() == mouseButtonCount, "mouseButtonNames needs one name per MouseButton.");
    }

    std::string_view toString(MouseButton button) {
        return mouseButtonNames[static_cast<std::size_t>(button)];
    }
}
