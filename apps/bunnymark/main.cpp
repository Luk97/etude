#include <etude/assets/qoi.h>
#include <etude/core/image.h>
#include <etude/core/log.h>
#include <etude/runtime/application.h>
#include <etude/runtime/frame_stats.h>
#include <etude/ui/context.h>
#include <etude/ui/modifier.h>
#include <etude/ui/row.h>
#include <etude/ui/text.h>
#include <etude/ui/theme.h>
#include <etude/ui/widgets/button.h>
#include <etude/ui/widgets/checkbox.h>
#include <etude/ui/widgets/panel.h>
#include <etude/ui/widgets/slider.h>
#include <etude/ui/widgets/text_field.h>

#include <algorithm>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <format>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace {

    /// @brief Pulls the bunnies down, in pixels per second squared.
    constexpr float gravity = 1800.0f;

    /// @brief How many bunnies join per frame while the left mouse button is held, until the panel Hasen changes it.
    constexpr int defaultBunniesPerFrame = 250;

    /// @brief How many bunnies a click on the button of the panel Hasen adds.
    constexpr int bunniesPerClick = 1000;

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

    /// @brief Reads a whole number that is not negative, or nothing if the text holds anything else.
    std::optional<int> readCount(std::string_view text) {
        int count = 0;
        const char* const end = text.data() + text.size();
        const auto [stop, error] = std::from_chars(text.data(), end, count);
        if (error != std::errc{} || stop != end || count < 0) {
            return std::nullopt;
        }
        return count;
    }

    /// @brief The classic benchmark for sprite renderers. While the left mouse button is held, bunnies jump out of the
    /// cursor, fall and bounce off the edges of the window. Releasing the button logs how many there are.
    ///
    /// Two panels show what etude.ui can do: one measures the frames and controls the bunnies, the other lists what
    /// happened and scrolls with the wheel.
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
            // A press that starts on the UI belongs to it until the button comes up, so it adds no bunnies, even where
            // the mouse moves meanwhile.
            if (input.held(etude::MouseButton::Left) && !ui().wantsMouse()) {
                for (int i = 0; i < bunniesPerFrame; ++i) {
                    spawn(input.mousePosition());
                }
            }
            if (input.released(etude::MouseButton::Left) && !ui().wantsMouse()) {
                etude::logInfo("{} bunnies", bunnies.size());
                events.push_back(std::format("{} Hasen", bunnies.size()));
            }
        }

        /// @brief Moves every bunny by one step, stretched by the tempo of the panel Hasen, unless its checkbox pauses
        /// them. The side edges reflect a bunny, the top edge stops its rise, and the bottom edge throws it back with
        /// less speed, sometimes with an extra jump.
        void onStep(etude::FixedTimestep::Duration step) override {
            if (paused) {
                return;
            }
            const float seconds = std::chrono::duration<float>(step).count() * tempo;
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

        /// @brief Shows the two panels, which move by their title and come to the front with a press. The calls find
        /// the building blocks and widgets of etude::ui by the type of ui, without naming the namespace.
        void onUi(etude::ui::Context& ui) override {
            // The measurements of the last second, as one line for the first panel.
            const etude::FrameStats& timing = frameStats();
            const std::string measured = std::format(
                "{:.0f} fps, CPU {:.2f} ms, GPU {:.2f} ms", timing.framesPerSecond, timing.cpuMilliseconds,
                timing.gpuMilliseconds
            );

            // The first panel shows the measurements and the number of bunnies, and it controls the bunnies.
            panel(ui, "Hasen", {8.0f, 8.0f}, {232.0f, 128.0f}, [&] {
                text(ui, measured);
                text(ui, std::format("{} Hasen", bunnies.size()));
                row(ui, etude::ui::Modifier().spacedBy(etude::ui::theme::gap), [&] {
                    if (button(ui, std::format("+{}", bunniesPerClick)).clicked) {
                        const etude::Size size = windowSize();
                        for (int i = 0; i < bunniesPerClick; ++i) {
                            spawn({static_cast<float>(size.width) / 2.0f, static_cast<float>(size.height) / 2.0f});
                        }
                        events.push_back(std::format("+{} Hasen", bunniesPerClick));
                    }
                    if (button(ui, "Leeren").clicked) {
                        bunnies.clear();
                        events.emplace_back("Geleert");
                    }
                });
                if (checkbox(ui, "Pause", paused).clicked) {
                    events.emplace_back(paused ? "Pause" : "Weiter");
                }
                slider(ui, "Tempo", tempo, 0.0f, 2.0f);

                // The field takes any text, and the bunnies per frame follow it whenever it reads as another count.
                textField(ui, "Hasen pro Frame", bunniesPerFrameText);
                const std::optional<int> count = readCount(bunniesPerFrameText);
                if (count && *count != bunniesPerFrame) {
                    bunniesPerFrame = *count;
                    events.push_back(std::format("{} Hasen pro Frame", bunniesPerFrame));
                }
            });

            // The second panel lists what happened, oldest first, and scrolls once the list outgrows it.
            panel(ui, "Protokoll", {248.0f, 8.0f}, {140.0f, 128.0f}, [&] {
                for (const std::string& event : events) {
                    text(ui, event);
                }
            });
        }

        void onDraw(etude::RenderList& list) override {
            for (const Bunny& bunny : bunnies) {
                list.sprites.push_back(bunny.sprite);
            }
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

        bool paused = false;
        float tempo = 1.0f;
        int bunniesPerFrame = defaultBunniesPerFrame;
        std::string bunniesPerFrameText = std::to_string(defaultBunniesPerFrame);

        /// @brief What happened, oldest first, for the second panel.
        std::vector<std::string> events;

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
