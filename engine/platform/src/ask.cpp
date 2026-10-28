#include <etude/platform/ask.h>

#include "wide_text.h"

#include <string>

#include <windows.h>

namespace etude {

    Answer ask(const Window& owner, std::string_view title, std::string_view question) {
        const auto window = static_cast<HWND>(owner.nativeHandles().window);
        const std::wstring text = toWide(question);
        const std::wstring caption = toWide(title);
        switch (MessageBoxW(window, text.c_str(), caption.c_str(), MB_YESNOCANCEL | MB_ICONQUESTION)) {
            case IDYES:
                return Answer::Yes;
            case IDNO:
                return Answer::No;
            default:
                return Answer::Cancel;
        }
    }
}
