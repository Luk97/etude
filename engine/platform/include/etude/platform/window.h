#pragma once

#include <etude/math/size.h>
#include <etude/platform/input.h>

#include <memory>
#include <string_view>

namespace etude {

    /// @brief Untyped Win32 handles of a window: the instance of the program module and the window itself.
    /// A graphics backend needs them to create the surface it draws on.
    struct NativeHandles {
        void* instance = nullptr;
        void* window = nullptr;
    };

    /// @brief A native window with title bar, frame and close button, built on the Win32 API.
    class Window {
    public:
        /// @brief Opens a window with a UTF-8 title and a client area of width by height physical pixels.
        Window(std::string_view title, int width, int height);

        /// @brief Closes the window. Defined in window.cpp, because the unique pointer needs the complete Native
        /// to delete it.
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        /// @brief Handles all pending messages of the calling thread without waiting for new ones.
        /// Call it once per frame, otherwise Windows marks the window as not responding.
        void pollEvents();

        /// @brief Returns true once the user has asked to close the window, for example with the close button.
        bool shouldClose() const;

        /// @brief Replaces the text in the title bar with UTF-8 text.
        void setTitle(std::string_view title);

        /// @brief Returns the size of the client area in physical pixels, 0 x 0 while the window is minimized.
        Size clientSize() const;

        /// @brief Returns the keyboard and mouse state of the current frame.
        const Input& input() const;

        /// @brief Returns the native handles of the window, which only a graphics backend should need.
        NativeHandles nativeHandles() const;

    private:
        /// @brief Win32 state and callbacks of the window, defined in window.cpp
        struct Native;

        std::unique_ptr<Native> native;
    };
}
