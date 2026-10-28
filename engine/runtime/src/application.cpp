#include <etude/runtime/application.h>

#include <etude/core/clock.h>
#include <etude/core/log.h>
#include <etude/platform/ask.h>
#include <etude/platform/file_dialog.h>
#include <etude/render2d/pixel_font.h>
#include <etude/rendering/vulkan/vulkan_renderer.h>
#include <etude/runtime/motion_system.h>
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
          textureLookup([this](const std::string& path) { return textureCache.get(path); }),
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

        while (true) {
            window.pollEvents();

            // A request to close ends the loop, unless the game keeps the window open, such as to ask about unsaved
            // changes first.
            if (window.shouldClose()) {
                if (onClose()) {
                    break;
                }
                window.keepOpen();
            }

            // Measured without the window messages, which stop the loop while the window is dragged. The UI starts its
            // frame first, so that onFrame can already ask whether it wants the mouse.
            const Clock game;
            const Input& input = window.input();
            uiContext.beginFrame(input, window.clientSize(), uiScale(window.dpiScale()));
            onFrame(input);

            // The fixed timestep decides how many simulation steps this frame catches up on, none or several. While the
            // simulation is off, the time passes all the same, so that no steps pile up for when it starts again.
            const int dueSteps = timestep.advance(frameClock.restart());
            if (simulating) {
                for (int i = 0; i < dueSteps; ++i) {
                    onStep(timestep.step());
                    moveScene(sceneWorld, timestep.step());
                }
                steps += dueSteps;
            }

            ++frames;

            // The game describes its UI before anything is drawn, so that the drawing already shows what the UI changed
            // and no batch of the game can point to sprites that the UI changes afterwards.
            onUi(uiContext);
            uiContext.endFrame();

            // The sprites and batches start anew in every frame, the sprites of the world first, which fill the
            // viewport or the whole window. A Camera in the world points the camera anew in every frame, without one it
            // stays where the game left it.
            const Size screen = window.clientSize();
            const Rect wholeWindow{
                .size = {static_cast<float>(screen.width), static_cast<float>(screen.height)},
            };
            const Rect area = viewport.value_or(wholeWindow);
            renderList.sprites.clear();
            renderList.batches.clear();
            drawScene(sceneWorld, area.size, textureLookup, renderList);
            onDraw(renderList);
            const std::chrono::nanoseconds gameTime = game.elapsed();

            // All sprites go through the camera as the first batch, cut off at the edges of a viewport, and the batches
            // of the game and then those of the UI cover them.
            const DrawBatch worldBatch{
                .viewProjection = renderList.camera.viewProjection(screen, area),
                .clip = viewport,
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
                showTitle();
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

    bool Application::onClose() {
        return true;
    }

    void Application::onDraw(RenderList&) {}

    void Application::setFramesPerSecond(int framesPerSecond) {
        limiter.setFramesPerSecond(framesPerSecond);
    }

    void Application::setClearColor(Color color) {
        renderer->setClearColor(color);
    }

    void Application::setTitle(std::string_view text) {
        title = text;
        showTitle();
    }

    void Application::setViewport(std::optional<Rect> area) {
        viewport = area;
    }

    void Application::setSimulating(bool on) {
        simulating = on;
    }

    void Application::setAssetFolder(std::filesystem::path folder) {
        textureCache.setAssetFolder(std::move(folder));
    }

    Answer Application::ask(std::string_view question) {
        return etude::ask(window, title, question);
    }

    std::optional<std::filesystem::path> Application::chooseFileToOpen(
        const std::filesystem::path& folder,
        const FileFilter& filter
    ) {
        return openFileDialog(window, folder, filter);
    }

    std::optional<std::filesystem::path> Application::chooseFileToSave(
        const std::filesystem::path& folder,
        std::string_view name,
        const FileFilter& filter
    ) {
        return saveFileDialog(window, folder, name, filter);
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

    const TextureLookup& Application::textures() const {
        return textureLookup;
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

    void Application::showTitle() {
        // Before the first full second, there are no numbers yet.
        if (stats.framesPerSecond == 0.0) {
            window.setTitle(title);
            return;
        }
        window.setTitle(
            std::format("{} | {:.0f} fps | {:.0f} steps/s", title, stats.framesPerSecond, stats.stepsPerSecond)
        );
    }
}
