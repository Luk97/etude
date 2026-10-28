#pragma once

#include <etude/platform/answer.h>
#include <etude/platform/window.h>

#include <string_view>

namespace etude {

    /// @brief Shows the question in a small window of Windows above the window, with the title in its bar and the
    /// buttons Yes, No and Cancel in the language of Windows, and waits for the answer. Closing the small window counts
    /// as Cancel.
    Answer ask(const Window& owner, std::string_view title, std::string_view question);
}
