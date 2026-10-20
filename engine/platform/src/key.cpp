#include <etude/platform/key.h>

#include <array>

namespace etude {

    namespace {

        /// @brief The name of each key, in the order of Key.
        constexpr auto keyNames = std::to_array<std::string_view>(
            {"A",       "B",   "C",         "D",      "E",     "F",      "G",    "H",     "I",  "J",    "K",
             "L",       "M",   "N",         "O",      "P",     "Q",      "R",    "S",     "T",  "U",    "V",
             "W",       "X",   "Y",         "Z",      "0",     "1",      "2",    "3",     "4",  "5",    "6",
             "7",       "8",   "9",         "Space",  "Enter", "Escape", "Left", "Right", "Up", "Down", "Shift",
             "Control", "Alt", "Backspace", "Delete", "Tab",   "Home",   "End"}
        );
        static_assert(keyNames.size() == keyCount, "keyNames needs one name per Key, in the same order.");
    }

    std::string_view toString(Key key) {
        return keyNames[static_cast<std::size_t>(key)];
    }
}
