#include <etude/runtime/application.h>

#include <etude/core/clock.h>
#include <etude/core/log.h>
#include <etude/render2d/pixel_font.h>
#include <etude/rendering/vulkan/vulkan_renderer.h>
#include <etude/runtime/render_system.h>

#include <algorithm>
#include <chrono>
#include <cmath>
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

        /// @brief Returns the scale of the UI on a display with the given scale. The pixel font is half as high as
        /// ordinary text, so the UI takes twice the scale of the display, rounded to a whole number so that the font
        /// stays sharp: 2 at 96 dpi, 3 at 144 dpi and 4 at 192 dpi.
        int uiScale(float dpiScale) {
            return std::max(1, static_cast<int>(std::lround(2.0f * dpiScale)));
        }
    }

    Application::Application(std::string_view title, int width, int height, std::filesystem::path assetFolder)
        : title(title), window(title, width, height), renderer(createVulkanRenderer(window)),
          textureCache(*renderer, std::move(assetFolder)),
          uiContext(PixelFont{renderer->createTexture(PixelFont::makeAtlas())}), timestep(stepsPerSecond),
          limiter(initialFramesPerSecond) {
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

            // Measured without the window messages, which stop the loop while the window is dragged. The UI starts its
            // frame first, so that onFrame can already ask whether it wants the mouse.
            const Clock game;
            const Input& input = window.input();
            uiContext.beginFrame(input, window.clientSize(), uiScale(window.dpiScale()));
            onFrame(input);

            // The fixed timestep decides how many simulation steps this frame catches up on, none or several.
            const int dueSteps = timestep.advance(frameClock.restart());
            for (int i = 0; i < dueSteps; ++i) {
                onStep(timestep.step());
            }
            steps += dueSteps;
            ++frames;

            // The game describes its UI before anything is drawn, so that the drawing already shows what the UI changed
            // and no batch of the game can point to sprites that the UI changes afterwards.
            onUi(uiContext);
            uiContext.endFrame();

            // The sprites and batches start anew in every frame, the sprites of the world first. A Camera in the world
            // points the camera anew in every frame, without one it stays where the game left it.
            renderList.sprites.clear();
            renderList.batches.clear();
            drawScene(sceneWorld, window.clientSize(), lookup, renderList);
            onDraw(renderList);
            const std::chrono::nanoseconds gameTime = game.elapsed();

            // All sprites go through the camera as the first batch, the batches of the game and then those of the UI
            // cover them.
            const DrawBatch worldBatch{
                .viewProjection = renderList.camera.viewProjection(window.clientSize()),
                .sprites = renderList.sprites,
            };
            renderList.batches.insert(renderList.batches.begin(), worldBatch);
            renderList.batches.append_range(uiContext.batches());
            renderer->render(renderList.batches);
            const FrameTimes times = renderer->frameTimes();
            cpuTime += gameTime + times.cpu;
            gpuTime += times.gpu;

            // Once a second, the measurements go into the stats of the game, the title and the log, and start anew.
            const double elapsed = secondClock.elapsedSeconds();
            if (elapsed >= 1.0) {
                stats = {
                    .framesPerSecond = frames / elapsed,
                    .stepsPerSecond = steps / elapsed,
                    .cpuMilliseconds = millisecondsPerFrame(cpuTime, frames),
                    .gpuMilliseconds = millisecondsPerFrame(gpuTime, frames),
                };
                window.setTitle(
                    std::format("{} | {:.0f} fps | {:.0f} steps/s", title, stats.framesPerSecond, stats.stepsPerSecond)
                );
                logInfo("CPU {:.2f} ms, GPU {:.2f} ms per frame", stats.cpuMilliseconds, stats.gpuMilliseconds);
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

    void Application::onUi(ui::Context&) {}

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

    const ui::Context& Application::ui() const {
        return uiContext;
    }

    const FrameStats& Application::frameStats() const {
        return stats;
    }
}
