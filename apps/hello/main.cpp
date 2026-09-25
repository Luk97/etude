#include <etude/core/clock.h>
#include <etude/core/fixed_timestep.h>
#include <etude/core/log.h>
#include <etude/platform/frame_limiter.h>
#include <etude/platform/window.h>

#include <cstddef>
#include <format>

namespace {

    /// @brief Logs every key and mouse button that went down or up since the last frame.
    void logInput(const etude::Input& input) {
        for (std::size_t i = 0; i < etude::keyCount; ++i) {
            const auto key = static_cast<etude::Key>(i);
            if (input.pressed(key)) {
                etude::logInfo("{} pressed", etude::toString(key));
            }
            if (input.released(key)) {
                etude::logInfo("{} released", etude::toString(key));
            }
        }

        const etude::Vec2 mouse = input.mousePosition();
        for (std::size_t i = 0; i < etude::mouseButtonCount; ++i) {
            const auto button = static_cast<etude::MouseButton>(i);
            if (input.pressed(button)) {
                etude::logInfo("{} mouse button pressed at ({}, {})", etude::toString(button), mouse.x, mouse.y);
            }
            if (input.released(button)) {
                etude::logInfo("{} mouse button released at ({}, {})", etude::toString(button), mouse.x, mouse.y);
            }
        }
    }

    /// @brief Switches the frame limit to 30, 60 or 144 frames per second with the keys 1, 2 and 3.
    void chooseFrameRate(const etude::Input& input, etude::FrameLimiter& limiter) {
        if (input.pressed(etude::Key::Digit1)) {
            limiter.setFramesPerSecond(30);
        }
        if (input.pressed(etude::Key::Digit2)) {
            limiter.setFramesPerSecond(60);
        }
        if (input.pressed(etude::Key::Digit3)) {
            limiter.setFramesPerSecond(144);
        }
    }
}

int main() {
    const etude::Clock clock;
    etude::Window window("ÉTUDE", 1280, 720);
    etude::logInfo("Window open after {:.1f} ms", clock.elapsedSeconds() * 1000.0);

    etude::FixedTimestep timestep(60);
    etude::FrameLimiter limiter(60);
    etude::Clock frameClock;
    etude::Clock secondClock;
    int frames = 0;
    int steps = 0;

    while (!window.shouldClose()) {
        window.pollEvents();
        logInput(window.input());
        chooseFrameRate(window.input(), limiter);

        // Once there is a game, it advances its simulation by timestep.step() for every due step.
        steps += timestep.advance(frameClock.restart());
        ++frames;

        const double elapsed = secondClock.elapsedSeconds();
        if (elapsed >= 1.0) {
            window.setTitle(std::format("ÉTUDE | {:.0f} fps | {:.0f} steps/s", frames / elapsed, steps / elapsed));
            frames = 0;
            steps = 0;
            secondClock.reset();
        }

        limiter.wait();
    }

    etude::logInfo("Window closed after {:.1f} s", clock.elapsedSeconds());
    return 0;
}
