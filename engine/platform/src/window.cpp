#include <etude/platform/window.h>

#include <etude/core/log.h>

#include <cstddef>
#include <cstdlib>
#include <string>

#include <windows.h>

namespace etude {

    struct Window::Native {
        HWND handle = nullptr;
        bool closeRequested = false;

        /// @brief Receives every message that Windows sends to the window.
        static LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    };

    namespace {

        /// @brief Converts UTF-8 to UTF-16, the encoding of the wide-character Win32 functions.
        std::wstring toWide(std::string_view text) {
            const int size = static_cast<int>(text.size());
            const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), size, nullptr, 0);
            std::wstring wide(static_cast<std::size_t>(length), L'\0');
            MultiByteToWideChar(CP_UTF8, 0, text.data(), size, wide.data(), length);
            return wide;
        }

        /// @brief Prepares the process for its windows and returns the name of the window class they share.
        /// Turns on per-monitor DPI awareness, so that Windows does not stretch the windows blurrily on scaled
        /// displays.
        const wchar_t* prepareProcess(WNDPROC windowProcedure) {
            SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

            WNDCLASSEXW windowClass{
                .cbSize = sizeof(windowClass),
                .lpfnWndProc = windowProcedure,
                .hInstance = GetModuleHandleW(nullptr),
                .hCursor = LoadCursorW(nullptr, IDC_ARROW),
                .lpszClassName = L"EtudeWindow"
            };
            RegisterClassExW(&windowClass);

            return windowClass.lpszClassName;
        }
    }

    LRESULT CALLBACK Window::Native::windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_NCCREATE: {
                // The first message carries the Native pointer passed to CreateWindowExW.
                const auto* creation = reinterpret_cast<const CREATESTRUCTW*>(lParam);
                SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(creation->lpCreateParams));
                break;
            }

            case WM_CLOSE: {
                // Only note the request. Skipping DefWindowProcW keeps the window alive until ~Window destroys it.
                auto* native = reinterpret_cast<Native*>(GetWindowLongPtrW(window, GWLP_USERDATA));
                native->closeRequested = true;
                return 0;
            }
        }

        return DefWindowProcW(window, message, wParam, lParam);
    }

    Window::Window(std::string_view title, int width, int height) : native(std::make_unique<Native>()) {

        // A function-local static is initialized only once, so later windows reuse the registered class.
        static const wchar_t* const windowClass = prepareProcess(Native::windowProcedure);

        // CreateWindowExW expects the outer size, so add title bar and frame as they appear at the system DPI.
        RECT bounds{0, 0, width, height};
        AdjustWindowRectExForDpi(&bounds, WS_OVERLAPPEDWINDOW, FALSE, 0, GetDpiForSystem());

        native->handle = CreateWindowExW(
            0, windowClass, toWide(title).c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
            bounds.right - bounds.left, bounds.bottom - bounds.top, nullptr, nullptr, GetModuleHandleW(nullptr),
            native.get()
        );
        if (native->handle == nullptr) {
            logFatal("Could not create the window, Win32 error {}", GetLastError());
            std::abort();
        }
    }

    Window::~Window() {
        DestroyWindow(native->handle);
    }

    void Window::pollEvents() {
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    bool Window::shouldClose() const {
        return native->closeRequested;
    }
}
