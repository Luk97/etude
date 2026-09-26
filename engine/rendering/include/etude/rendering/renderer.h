#pragma once

#include <etude/math/color.h>

namespace etude {

    /// @brief Draws the frames of a game into its window.
    /// The graphics API behind it is an implementation detail: ETUDE implements the renderer with Vulkan. Another
    /// API such as OpenGL could offer the same interface without changes to the rest of the engine.
    class Renderer {
    public:
        virtual ~Renderer() = default;

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        /// @brief Draws the next frame into the window. Does nothing while the window has no area, for example
        /// while it is minimized.
        virtual void render() = 0;

        /// @brief Sets the color that fills the window at the start of every frame.
        virtual void setClearColor(Color color) = 0;

    protected:
        Renderer() = default;
    };
}
