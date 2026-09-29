#include <etude/core/log.h>
#include <etude/runtime/application.h>
#include <etude/scene/components.h>
#include <etude/scene/scene_file.h>

#include <array>
#include <cstddef>
#include <filesystem>
#include <random>
#include <utility>

namespace {

    const std::filesystem::path scenePath = TESTBED_ASSETS "/scenes/testbed.json";

    /// @brief How many bunnies a press of the space bar adds, to measure the frame with many entities.
    constexpr int bunniesPerPress = 1000;

    constexpr std::array bunnyTextures{
        "sprites/bunny-white.qoi",
        "sprites/bunny-brown.qoi",
        "sprites/bunny-gray.qoi",
        "sprites/bunny-blue.qoi",
    };

    /// @brief Shows the testbed scene and saves it again. The space bar adds bunnies at random places with a random
    /// rotation and layer, and S writes the world back into the scene file.
    class Testbed : public etude::Application {
    public:
        Testbed() : Application("ÉTUDE Testbed", 1280, 720, TESTBED_ASSETS) {}

        /// @brief Replaces the world by the testbed scene. Returns false after logging why if that fails.
        bool load() {
            auto loaded = etude::loadScene(scenePath, registry());
            if (!loaded) {
                etude::logError("{}", loaded.error());
                return false;
            }
            world() = std::move(*loaded);
            etude::logInfo("{} entities loaded, space adds {} bunnies, S saves", world().size(), bunniesPerPress);
            return true;
        }

    protected:
        void onFrame(const etude::Input& input) override {
            if (input.pressed(etude::Key::Space)) {
                for (int i = 0; i < bunniesPerPress; ++i) {
                    addBunny();
                }
                etude::logInfo("{} entities", world().size());
            }
            if (input.pressed(etude::Key::S)) {
                save();
            }
        }

    private:
        void addBunny() {
            const etude::Vec2 position{x(random), y(random)};
            const etude::Transform2D transform{
                .position = position,
                .rotation = angle(random),
            };
            const etude::Entity bunny = world().create();
            world().add(bunny, transform);
            world().add(bunny, etude::SpriteRenderer{bunnyTextures[texture(random)], layer(random)});
        }

        void save() {
            if (const auto saved = etude::saveScene(scenePath, world(), registry()); !saved) {
                etude::logError("{}", saved.error());
                return;
            }
            etude::logInfo("{} entities saved to {}", world().size(), scenePath.string());
        }

        // A fixed seed, so that every run adds the same bunnies.
        std::mt19937 random{42};
        std::uniform_real_distribution<float> x{0.0f, 1280.0f};
        std::uniform_real_distribution<float> y{0.0f, 720.0f};
        std::uniform_real_distribution<float> angle{0.0f, 360.0f};
        std::uniform_int_distribution<std::size_t> texture{0, bunnyTextures.size() - 1};
        std::uniform_int_distribution<int> layer{0, 2};
    };
}

int main() {
    Testbed testbed;
    if (!testbed.load()) {
        return 1;
    }
    testbed.run();
    return 0;
}
