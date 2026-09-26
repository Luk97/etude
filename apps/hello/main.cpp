#include <etude/core/clock.h>
#include <etude/core/log.h>
#include <etude/runtime/application.h>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <numbers>

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

    /// @brief Demo that logs all input, switches the frame limit to 30, 60 or 144 frames per second with the keys 1, 2
    /// and 3, and lets the clear color wander through all hues.
    class Hello : public etude::Application {
    public:
        Hello() : Application("ÉTUDE", 1280, 720) {}

    protected:
        void onFrame(const etude::Input& input) override {
            logInput(input);
            if (input.pressed(etude::Key::Digit1)) {
                setFramesPerSecond(30);
            }
            if (input.pressed(etude::Key::Digit2)) {
                setFramesPerSecond(60);
            }
            if (input.pressed(etude::Key::Digit3)) {
                setFramesPerSecond(144);
            }
        }

        /// @brief Shifts red, green and blue by a third of a turn each, so that together they run through all hues
        /// about every six seconds.
        void onStep(etude::FixedTimestep::Duration step) override {
            time += std::chrono::duration<float>(step).count();
            const float third = 2.0f * std::numbers::pi_v<float> / 3.0f;
            setClearColor({
                .r = 0.5f + 0.5f * std::sin(time),
                .g = 0.5f + 0.5f * std::sin(time + third),
                .b = 0.5f + 0.5f * std::sin(time + 2.0f * third),
            });
        }

    private:
        float time = 0.0f;
    };
}

int main() {
    const etude::Clock clock;
    Hello hello;
    etude::logInfo("Window open after {:.1f} ms", clock.elapsedSeconds() * 1000.0);

    hello.run();

    etude::logInfo("Window closed after {:.1f} s", clock.elapsedSeconds());
    return 0;
}
