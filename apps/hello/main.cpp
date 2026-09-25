#include <etude/core/clock.h>
#include <etude/core/log.h>
#include <etude/runtime/application.h>

#include <cstddef>

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

    /// @brief Demo that logs all input and switches the frame limit to 30, 60 or 144 frames per second with the keys
    /// 1, 2 and 3.
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
