#pragma once

#include <etude/core/image.h>
#include <etude/math/color.h>
#include <etude/math/mat3.h>
#include <etude/rendering/sprite.h>
#include <etude/rendering/texture_id.h>

#include <span>

namespace etude {

    /// @brief Draws the frames of a game into its window.
    /// The graphics API behind it is an implementation detail: ETUDE implements the renderer with Vulkan. Another
    /// API such as OpenGL could offer the same interface without changes to the rest of the engine.
    class Renderer {
    public:
        virtual ~Renderer() = default;

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        /// @brief Draws the sprites over the clear color into the window. Later sprites cover earlier ones, and the
        /// matrix maps their world coordinates to clip space. Does nothing while the window has no area, for example
        /// while it is minimized.
        virtual void render(const Mat3& viewProjection, std::span<const Sprite> sprites) = 0;

        /// @brief Sets the color that fills the window at the start of every frame.
        virtual void setClearColor(Color color) = 0;

        /// @brief Copies the image to the GPU and returns the id under which it can be drawn. Waits until the copy is
        /// done, so textures are created while loading and not in every frame.
        virtual TextureId createTexture(const Image& image) = 0;

    protected:
        Renderer() = default;
    };
}
