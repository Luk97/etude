#include <etude/platform/window.h>

#include <etude/core/log.h>

#include <cstddef>
#include <cstdlib>
#include <optional>
#include <string>
#include <utility>

#include <windows.h>
#include <windowsx.h>

namespace etude {

    struct Window::Native {
        HWND handle = nullptr;
        bool closeRequested = false;
        Input input;

        /// @brief Receives every message that Windows sends to the window.
        static LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

        /// @brief Returns the Native that WM_NCCREATE stored in the window. Only WM_GETMINMAXINFO arrives before
        /// WM_NCCREATE, so every other message may use it.
        static Native& from(HWND window);
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

        /// @brief Maps a Win32 virtual-key code to a Key, or to nothing for keys that ETUDE does not support.
        std::optional<Key> toKey(WPARAM virtualKey) {
            if (virtualKey >= 'A' && virtualKey <= 'Z') {
                return static_cast<Key>(std::to_underlying(Key::A) + (virtualKey - 'A'));
            }
            if (virtualKey >= '0' && virtualKey <= '9') {
                return static_cast<Key>(std::to_underlying(Key::Digit0) + (virtualKey - '0'));
            }
            switch (virtualKey) {
                case VK_SPACE:
                    return Key::Space;
                case VK_RETURN:
                    return Key::Enter;
                case VK_ESCAPE:
                    return Key::Escape;
                case VK_LEFT:
                    return Key::Left;
                case VK_RIGHT:
                    return Key::Right;
                case VK_UP:
                    return Key::Up;
                case VK_DOWN:
                    return Key::Down;
                case VK_SHIFT:
                    return Key::Shift;
                case VK_CONTROL:
                    return Key::Control;
                case VK_MENU:
                    return Key::Alt;
                default:
                    return std::nullopt;
            }
        }

        /// @brief Returns the mouse button of a button message such as WM_LBUTTONDOWN.
        MouseButton toMouseButton(UINT message) {
            switch (message) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    return MouseButton::Left;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    return MouseButton::Right;
                default:
                    return MouseButton::Middle;
            }
        }

        /// @brief Reads the cursor position in client pixels from the parameter of a mouse message.
        Vec2 toPosition(LPARAM lParam) {
            return {static_cast<float>(GET_X_LPARAM(lParam)), static_cast<float>(GET_Y_LPARAM(lParam))};
        }
    }

    Window::Native& Window::Native::from(HWND window) {
        return *reinterpret_cast<Native*>(GetWindowLongPtrW(window, GWLP_USERDATA));
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
                from(window).closeRequested = true;
                return 0;
            }

            case WM_SYSCOMMAND:
                // Releasing Alt would switch to menu mode, where the next key goes to the window menu, not the game.
                if ((wParam & 0xFFF0) == SC_KEYMENU) {
                    return 0;
                }
                break;

            case WM_KILLFOCUS:
                from(window).input.onFocusLost();
                break;

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                if (const auto key = toKey(wParam)) {
                    from(window).input.onKeyDown(*key);
                }
                break;

            case WM_KEYUP:
            case WM_SYSKEYUP:
                if (const auto key = toKey(wParam)) {
                    from(window).input.onKeyUp(*key);
                }
                break;

            case WM_MOUSEMOVE:
                from(window).input.onMouseMove(toPosition(lParam));
                break;

            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
                // Capturing the mouse delivers the button-up message even if the cursor has left the window by then.
                SetCapture(window);
                from(window).input.onMouseMove(toPosition(lParam));
                from(window).input.onMouseButtonDown(toMouseButton(message));
                break;

            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP:
                from(window).input.onMouseMove(toPosition(lParam));
                from(window).input.onMouseButtonUp(toMouseButton(message));
                if ((wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)) == 0) {
                    ReleaseCapture();
                }
                break;
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
        native->input.beginFrame();

        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    bool Window::shouldClose() const {
        return native->closeRequested;
    }

    void Window::setTitle(std::string_view title) {
        SetWindowTextW(native->handle, toWide(title).c_str());
    }

    const Input& Window::input() const {
        return native->input;
    }

    NativeHandles Window::nativeHandles() const {
        return {GetModuleHandleW(nullptr), native->handle};
    }
}
