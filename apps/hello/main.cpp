#include <etude/core/clock.h>
#include <etude/core/image.h>
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

    /// @brief An 8 x 8 checkerboard of white and gray texels. Sampled with the nearest texel, its squares keep sharp
    /// edges at any size.
    etude::Image checkerboard() {
        etude::Image image{
            .width = 8,
            .height = 8,
        };
        for (int y = 0; y < image.height; ++y) {
            for (int x = 0; x < image.width; ++x) {
                const bool white = (x + y) % 2 == 0;
                image.pixels.push_back(white ? etude::Pixel{255, 255, 255, 255} : etude::Pixel{128, 128, 128, 255});
            }
        }
        return image;
    }

    /// @brief A 16 x 16 disc of translucent red on a transparent background, to show alpha blending.
    etude::Image disc() {
        etude::Image image{
            .width = 16,
            .height = 16,
        };
        for (int y = 0; y < image.height; ++y) {
            for (int x = 0; x < image.width; ++x) {
                const etude::Vec2 offset{static_cast<float>(x) - 7.5f, static_cast<float>(y) - 7.5f};
                const bool inside = offset.lengthSquared() <= 64.0f;
                image.pixels.push_back(inside ? etude::Pixel{255, 64, 64, 160} : etude::Pixel{});
            }
        }
        return image;
    }

    /// @brief Demo that logs all input, switches the frame limit to 30, 60 or 144 frames per second with the keys 1, 2
    /// and 3, lets the clear color wander through all hues and draws a checkerboard with eight translucent discs
    /// circling around it.
    class Hello : public etude::Application {
    public:
        Hello() : Application("ÉTUDE", 1280, 720), board(createTexture(checkerboard())), dot(createTexture(disc())) {}

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

        /// @brief Draws the checkerboard first and the discs over it. All sprites go into one draw, although they use
        /// two textures.
        void onDraw(etude::RenderList& list) override {
            const etude::Vec2 center{640.0f, 360.0f};
            list.sprites.push_back({
                .position = center - etude::Vec2{128.0f, 128.0f},
                .size = {256.0f, 256.0f},
                .texture = board,
            });
            for (int i = 0; i < 8; ++i) {
                const float angle = time + static_cast<float>(i) * std::numbers::pi_v<float> / 4.0f;
                const etude::Vec2 orbit{200.0f * std::cos(angle), 200.0f * std::sin(angle)};
                list.sprites.push_back({
                    .position = center + orbit - etude::Vec2{32.0f, 32.0f},
                    .size = {64.0f, 64.0f},
                    .texture = dot,
                });
            }
        }

    private:
        etude::TextureId board;
        etude::TextureId dot;
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
