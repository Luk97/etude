#pragma once

#include <etude/core/image.h>
#include <etude/math/color.h>
#include <etude/rendering/draw_batch.h>
#include <etude/rendering/texture_id.h>

#include <chrono>
#include <span>

namespace etude {

    /// @brief How long the renderer worked on a frame: on the CPU without waiting for the GPU or the display, and on
    /// the GPU while it drew the scene.
    struct FrameTimes {
        std::chrono::nanoseconds cpu{};
        std::chrono::nanoseconds gpu{};
    };

    /// @brief Draws the frames of a game into its window.
    /// The graphics API behind it is an implementation detail: ETUDE implements the renderer with Vulkan. Another
    /// API such as OpenGL could offer the same interface without changes to the rest of the engine.
    class Renderer {
    public:
        virtual ~Renderer() = default;

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        /// @brief Draws the batches one after another over the clear color into the window, so later batches cover
        /// earlier ones like later sprites within a batch. Does nothing while the window has no area, for example
        /// while it is minimized.
        virtual void render(std::span<const DrawBatch> batches) = 0;

        /// @brief Sets the color that fills the window at the start of every frame.
        virtual void setClearColor(Color color) = 0;

        /// @brief Copies the image to the GPU and returns the id under which it can be drawn. Waits until the copy is
        /// done, so the frame that creates a texture takes that much longer.
        virtual TextureId createTexture(const Image& image) = 0;

        /// @brief Returns the times of the last frame. The GPU time belongs to an earlier frame, because the renderer
        /// can only read it once the GPU has finished that frame.
        virtual FrameTimes frameTimes() const = 0;

    protected:
        Renderer() = default;
    };
}
