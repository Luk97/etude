#include <etude/core/clock.h>
#include <etude/core/log.h>
#include <etude/platform/window.h>

int main() {
    const etude::Clock clock;
    etude::Window window("ÉTUDE", 1280, 720);
    etude::logInfo("Window open after {:.1f} ms", clock.elapsedSeconds() * 1000.0);

    while (!window.shouldClose()) {
        window.pollEvents();
    }

    etude::logInfo("Window closed after {:.1f} s", clock.elapsedSeconds());
    return 0;
}
