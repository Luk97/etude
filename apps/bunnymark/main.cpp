#include <etude/assets/qoi.h>
#include <etude/core/image.h>
#include <etude/core/log.h>
#include <etude/render2d/pixel_font.h>
#include <etude/runtime/application.h>
#include <etude/ui/context.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <random>
#include <system_error>
#include <utility>
#include <vector>

namespace {

    /// @brief Pulls the bunnies down, in pixels per second squared.
    constexpr float gravity = 1800.0f;

    /// @brief How many bunnies join per frame while the left mouse button is held.
    constexpr int bunniesPerFrame = 250;

    /// @brief How many bunnies a click on the button of the test panel adds.
    constexpr int bunniesPerClick = 1000;

    /// @brief The dark background of the test panel, a little transparent, so that its text stays readable over the
    /// bunnies.
    constexpr etude::Color panelColor{
        .r = 0.05f,
        .g = 0.05f,
        .b = 0.08f,
        .a = 0.8f,
    };

    /// @brief A sprite that moves: gravity pulls it down, and the edges of the window throw it back.
    struct Bunny {
        etude::Sprite sprite;
        etude::Vec2 velocity;
    };

    /// @brief Returns the QOI files in the directory, sorted by name, so that every run hands out the textures alike.
    /// A missing directory gives an empty list, just like a directory without sprites.
    std::vector<std::filesystem::path> findSprites(const std::filesystem::path& directory) {
        std::vector<std::filesystem::path> files;
        std::error_code error;
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directory, error)) {
            if (entry.path().extension() == ".qoi") {
                files.push_back(entry.path());
            }
        }
        std::ranges::sort(files);
        return files;
    }

    /// @brief The classic benchmark for sprite renderers. While the left mouse button is held, bunnies jump out of the
    /// cursor, fall and bounce off the edges of the window. Releasing the button logs how many there are.
    class Bunnymark : public etude::Application {
    public:
        explicit Bunnymark(const std::vector<etude::Image>& images) : Application("ÉTUDE Bunnymark", 1280, 720) {
            for (const etude::Image& image : images) {
                kinds.push_back({
                    .size = {static_cast<float>(image.width), static_cast<float>(image.height)},
                    .texture = createTexture(image),
                });
            }
            etude::logInfo("{} bunny textures, hold the left mouse button to add bunnies", kinds.size());
        }

    protected:
        void onFrame(const etude::Input& input) override {
            // Test code for etude.ui: a panel with the number of bunnies and two buttons. A press that starts on the
            // panel belongs to the panel until the button comes up, so it adds no bunnies, even where the mouse moves
            // meanwhile.
            ui.beginFrame(input, windowSize(), 2);
            const etude::ui::Signal panel = ui.beginBox({
                .label = "Panel",
                .background = panelColor,
            });
            ui.label(std::format("{} Hasen", bunnies.size()));
            if (ui.button(std::format("+{}", bunniesPerClick))) {
                const etude::Size size = windowSize();
                for (int i = 0; i < bunniesPerClick; ++i) {
                    spawn({static_cast<float>(size.width) / 2.0f, static_cast<float>(size.height) / 2.0f});
                }
            }
            if (ui.button("Leeren")) {
                bunnies.clear();
            }
            ui.endBox();
            ui.endFrame();

            if (input.pressed(etude::MouseButton::Left)) {
                pressOnPanel = panel.rect.contains(input.mousePosition());
            }
            if (input.held(etude::MouseButton::Left) && !pressOnPanel) {
                for (int i = 0; i < bunniesPerFrame; ++i) {
                    spawn(input.mousePosition());
                }
            }
            if (input.released(etude::MouseButton::Left)) {
                etude::logInfo("{} bunnies", bunnies.size());
            }
        }

        /// @brief Moves every bunny by one step. The side edges reflect it, the top edge stops its rise, and the bottom
        /// edge throws it back with less speed, sometimes with an extra jump.
        void onStep(etude::FixedTimestep::Duration step) override {
            const float seconds = std::chrono::duration<float>(step).count();
            const etude::Size bounds = windowSize();
            for (Bunny& bunny : bunnies) {
                etude::Vec2& position = bunny.sprite.position;
                etude::Vec2& velocity = bunny.velocity;
                velocity.y += gravity * seconds;
                position = position + velocity * seconds;

                const float right = static_cast<float>(bounds.width) - bunny.sprite.size.x;
                const float bottom = static_cast<float>(bounds.height) - bunny.sprite.size.y;
                if (position.x > right) {
                    position.x = right;
                    velocity.x = -velocity.x;
                } else if (position.x < 0.0f) {
                    position.x = 0.0f;
                    velocity.x = -velocity.x;
                }
                if (position.y > bottom) {
                    position.y = bottom;
                    velocity.y *= -0.85f;
                    if (coinFlip(random)) {
                        velocity.y -= jump(random);
                    }
                } else if (position.y < 0.0f) {
                    position.y = 0.0f;
                    velocity.y = 0.0f;
                }
            }
        }

        void onDraw(etude::RenderList& list) override {
            for (const Bunny& bunny : bunnies) {
                list.sprites.push_back(bunny.sprite);
            }
            list.sprites.append_range(ui.sprites());
        }

    private:
        /// @brief Adds a bunny centered on the position. The bunnies take turns with the textures, so neighbors in the
        /// list draw with different ones.
        void spawn(etude::Vec2 position) {
            etude::Sprite sprite = kinds[bunnies.size() % kinds.size()];
            sprite.position = position - sprite.size * 0.5f;
            bunnies.push_back({
                .sprite = sprite,
                .velocity = {sideways(random), upwards(random)},
            });
        }

        /// @brief One sprite per texture with the size of its image. New bunnies are copies of these.
        std::vector<etude::Sprite> kinds;
        std::vector<Bunny> bunnies;

        /// @brief Test code for etude.ui. Draws in the coordinates of the world, which match those of the window as
        /// long as the camera stays in the top left corner at zoom 1.
        etude::ui::Context ui{etude::PixelFont{createTexture(etude::PixelFont::makeAtlas())}};
        bool pressOnPanel = false;

        // A fixed seed, so that every run moves the bunnies alike.
        std::mt19937 random{42};
        std::uniform_real_distribution<float> sideways{-300.0f, 300.0f};
        std::uniform_real_distribution<float> upwards{-600.0f, 0.0f};
        std::uniform_real_distribution<float> jump{0.0f, 360.0f};
        std::bernoulli_distribution coinFlip{0.5};
    };
}

int main() {
    std::vector<etude::Image> images;
    for (const std::filesystem::path& file : findSprites(BUNNYMARK_ASSETS)) {
        auto image = etude::loadQoi(file);
        if (!image) {
            etude::logError("{}", image.error());
            return 1;
        }
        images.push_back(std::move(*image));
    }
    if (images.empty()) {
        etude::logError("Bunnymark needs its sprites as QOI files in {}.", BUNNYMARK_ASSETS);
        return 1;
    }

    Bunnymark bunnymark(images);
    bunnymark.run();
    return 0;
}
