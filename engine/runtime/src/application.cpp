#include <etude/runtime/application.h>

#include <etude/core/clock.h>
#include <etude/core/log.h>
#include <etude/rendering/vulkan/vulkan_renderer.h>
#include <etude/runtime/render_system.h>

#include <chrono>
#include <format>
#include <string>
#include <utility>

namespace etude {

    namespace {

        constexpr int stepsPerSecond = 60;
        constexpr int initialFramesPerSecond = 60;

        /// @brief Converts the total of several frames into milliseconds per frame.
        double millisecondsPerFrame(std::chrono::nanoseconds total, int frames) {
            return std::chrono::duration<double, std::milli>(total).count() / frames;
        }
    }

    Application::Application(std::string_view title, int width, int height, std::filesystem::path assetFolder)
        : title(title), window(title, width, height), renderer(createVulkanRenderer(window)),
          textureCache(*renderer, std::move(assetFolder)), timestep(stepsPerSecond), limiter(initialFramesPerSecond) {
        addBuiltinComponents(componentTypes);
    }

    void Application::run() {
        Clock frameClock;
        Clock secondClock;
        int frames = 0;
        int steps = 0;
        std::chrono::nanoseconds cpuTime{};
        std::chrono::nanoseconds gpuTime{};
        const TextureLookup lookup = [this](const std::string& path) { return textureCache.get(path); };

        while (!window.shouldClose()) {
            window.pollEvents();

            // Measured without the window messages, which stop the loop while the window is dragged.
            const Clock game;
            onFrame(window.input());

            const int dueSteps = timestep.advance(frameClock.restart());
            for (int i = 0; i < dueSteps; ++i) {
                onStep(timestep.step());
            }
            steps += dueSteps;
            ++frames;

            // The sprites and batches start anew in every frame, the sprites of the world first. A Camera in the world
            // points the camera anew in every frame, without one it stays where the game left it.
            renderList.sprites.clear();
            renderList.batches.clear();
            drawScene(sceneWorld, window.clientSize(), lookup, renderList);
            onDraw(renderList);
            const std::chrono::nanoseconds gameTime = game.elapsed();

            // All sprites go through the camera as the first batch, the batches of the game cover them.
            const DrawBatch worldBatch{
                .viewProjection = renderList.camera.viewProjection(window.clientSize()),
                .sprites = renderList.sprites,
            };
            renderList.batches.insert(renderList.batches.begin(), worldBatch);
            renderer->render(renderList.batches);
            const FrameTimes times = renderer->frameTimes();
            cpuTime += gameTime + times.cpu;
            gpuTime += times.gpu;

            const double elapsed = secondClock.elapsedSeconds();
            if (elapsed >= 1.0) {
                window.setTitle(
                    std::format("{} | {:.0f} fps | {:.0f} steps/s", title, frames / elapsed, steps / elapsed)
                );
                logInfo(
                    "CPU {:.2f} ms, GPU {:.2f} ms per frame", millisecondsPerFrame(cpuTime, frames),
                    millisecondsPerFrame(gpuTime, frames)
                );
                frames = 0;
                steps = 0;
                cpuTime = {};
                gpuTime = {};
                secondClock.reset();
            }

            limiter.wait();
        }
    }

    void Application::onFrame(const Input&) {}

    void Application::onStep(FixedTimestep::Duration) {}

    void Application::onDraw(RenderList&) {}

    void Application::setFramesPerSecond(int framesPerSecond) {
        limiter.setFramesPerSecond(framesPerSecond);
    }

    void Application::setClearColor(Color color) {
        renderer->setClearColor(color);
    }

    TextureId Application::createTexture(const Image& image) {
        return renderer->createTexture(image);
    }

    Size Application::windowSize() const {
        return window.clientSize();
    }

    World& Application::world() {
        return sceneWorld;
    }

    ComponentRegistry& Application::registry() {
        return componentTypes;
    }
}
