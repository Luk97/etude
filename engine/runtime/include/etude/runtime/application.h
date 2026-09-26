#pragma once

#include <etude/core/color.h>
#include <etude/core/fixed_timestep.h>
#include <etude/platform/frame_limiter.h>
#include <etude/platform/input.h>
#include <etude/platform/window.h>
#include <etude/rendering/renderer.h>

#include <memory>
#include <string>
#include <string_view>

namespace etude {

    /// @brief Runs the game loop: handles the window messages, calls the game once per frame and once per fixed
    /// simulation step, shows frames and steps per second in the title and limits the frame rate.
    /// A game derives from Application and overrides onFrame and onStep, the loop itself stays the same.
    class Application {
    public:
        Application(std::string_view title, int width, int height);
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

        /// @brief Sets the frames per second.
        void setFramesPerSecond(int framesPerSecond);

        /// @brief Sets the color that fills the window at the start of every frame.
        void setClearColor(Color color);

    private:
        std::string title;
        Window window;
        std::unique_ptr<Renderer> renderer;
        FixedTimestep timestep;
        FrameLimiter limiter;
    };
}
