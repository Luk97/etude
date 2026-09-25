#include <etude/runtime/application.h>

#include <etude/core/clock.h>
#include <etude/vulkan/vulkan_renderer.h>

#include <format>

namespace etude {

    namespace {

        constexpr int stepsPerSecond = 60;
        constexpr int initialFramesPerSecond = 60;
    }

    Application::Application(std::string_view title, int width, int height)
        : title(title), window(title, width, height), renderer(createVulkanRenderer(window)), timestep(stepsPerSecond),
          limiter(initialFramesPerSecond) {}

    void Application::run() {
        Clock frameClock;
        Clock secondClock;
        int frames = 0;
        int steps = 0;

        while (!window.shouldClose()) {
            window.pollEvents();
            onFrame(window.input());

            const int dueSteps = timestep.advance(frameClock.restart());
            for (int i = 0; i < dueSteps; ++i) {
                onStep(timestep.step());
            }
            steps += dueSteps;
            ++frames;

            const double elapsed = secondClock.elapsedSeconds();
            if (elapsed >= 1.0) {
                window.setTitle(
                    std::format("{} | {:.0f} fps | {:.0f} steps/s", title, frames / elapsed, steps / elapsed)
                );
                frames = 0;
                steps = 0;
                secondClock.reset();
            }

            limiter.wait();
        }
    }

    void Application::onFrame(const Input&) {}

    void Application::onStep(FixedTimestep::Duration) {}

    void Application::setFramesPerSecond(int framesPerSecond) {
        limiter.setFramesPerSecond(framesPerSecond);
    }
}
