#pragma once

#include <etude/core/fixed_timestep.h>
#include <etude/math/color.h>
#include <etude/platform/frame_limiter.h>
#include <etude/platform/input.h>
#include <etude/platform/window.h>
#include <etude/render2d/render_list.h>
#include <etude/rendering/renderer.h>
#include <etude/runtime/frame_stats.h>
#include <etude/runtime/texture_cache.h>
#include <etude/scene/component_registry.h>
#include <etude/scene/world.h>
#include <etude/ui/context.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace etude {

    /// @brief Runs the game loop: handles the window messages, calls the game once per frame and once per fixed
    /// simulation step, draws the world of the running scene with the UI on top, shows frames and steps per second in
    /// the title and limits the frame rate.
    ///
    /// A game derives from Application and overrides onFrame, onStep, onUi and onDraw, which the loop calls in this
    /// order, and the loop itself stays the same. In every frame, the UI starts first, so that onFrame can already ask
    /// whether the UI wants the mouse or the keyboard. The UI scales with the display in whole steps, so that its pixel
    /// font stays sharp.
    class Application {
    public:
        /// @brief Opens the window. The textures of the world load from the asset folder, which is the working
        /// directory if left out.
        Application(std::string_view title, int width, int height, std::filesystem::path assetFolder = {});
        virtual ~Application() = default;

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        /// @brief Runs the game loop until the user closes the window.
        void run();

    protected:
        /// @brief Called once per frame with the input of that frame. Pressed and released refer to frames, so a
        /// reaction to a single key press belongs here. A frame can contain several simulation steps or none.
        virtual void onFrame(const Input& input);

        /// @brief Called once per simulation step, 60 times per second at any frame rate.
        virtual void onStep(FixedTimestep::Duration step);

        /// @brief Called once per frame after the simulation steps and before onDraw, to describe the UI of the frame
        /// with the building blocks and widgets of etude.ui, so that the drawing of the frame already shows what the UI
        /// changed. The UI lies in the pixels of the window above the world and the batches of the game.
        virtual void onUi(ui::Context& ui);

        /// @brief Called once per frame after onUi, right before the frame is drawn. The list already holds the sprites
        /// of the world, and the game may add its own on top of them. It may also move the camera, unless the world has
        /// a Camera, which points the camera anew in every frame. Batches that the game adds cover all sprites.
        virtual void onDraw(RenderList& list);

        /// @brief Sets the frames per second.
        void setFramesPerSecond(int framesPerSecond);

        /// @brief Sets the color that fills the window at the start of every frame.
        void setClearColor(Color color);

        /// @brief Copies the image to the GPU and returns the id under which it can be drawn.
        TextureId createTexture(const Image& image);

        /// @brief Returns the size of the drawing area of the window in pixels, 0 x 0 while it is minimized.
        Size windowSize() const;

        /// @brief Returns the entities and components of the running scene, which the loop draws in every frame.
        World& world();

        /// @brief Returns the component types that scenes can hold. The engine adds its own first, a game adds its
        /// own in its constructor, before it loads a scene.
        ComponentRegistry& registry();

        /// @brief Returns the UI, for example to ask in onFrame whether it wants the mouse or the keyboard.
        const ui::Context& ui() const;

        /// @brief Returns how fast the loop ran over the last full second.
        const FrameStats& frameStats() const;

    private:
        std::string title;
        Window window;
        std::unique_ptr<Renderer> renderer;
        TextureCache textureCache;
        ui::Context uiContext;
        ComponentRegistry componentTypes;
        World sceneWorld;
        RenderList renderList;
        FixedTimestep timestep;
        FrameLimiter limiter;
        FrameStats stats;
    };
}
