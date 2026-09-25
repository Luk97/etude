#include <etude/core/clock.h>
#include <etude/core/log.h>
#include <etude/platform/window.h>

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
}

int main() {
    const etude::Clock clock;
    etude::Window window("ÉTUDE", 1280, 720);
    etude::logInfo("Window open after {:.1f} ms", clock.elapsedSeconds() * 1000.0);

    while (!window.shouldClose()) {
        window.pollEvents();
        logInput(window.input());
    }

    etude::logInfo("Window closed after {:.1f} s", clock.elapsedSeconds());
    return 0;
}
