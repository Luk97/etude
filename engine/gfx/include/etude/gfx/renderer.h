#pragma once

namespace etude {

    /// @brief Draws the frames of a game into its window.
    /// The graphics API behind it is an implementation detail: ETUDE implements the renderer with Vulkan. Another
    /// API such as OpenGL could offer the same interface without changes to the rest of the engine.
    class Renderer {
    public:
        virtual ~Renderer() = default;

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

    protected:
        Renderer() = default;
    };
}
