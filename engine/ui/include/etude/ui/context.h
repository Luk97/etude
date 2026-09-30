#pragma once

#include <etude/math/color.h>
#include <etude/math/rect.h>
#include <etude/math/size.h>
#include <etude/platform/input.h>
#include <etude/render2d/pixel_font.h>
#include <etude/rendering/sprite.h>
#include <etude/ui/box.h>
#include <etude/ui/id.h>

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace etude::ui {

    /// @brief Builds the UI anew in every frame. Between beginFrame and endFrame, widgets describe themselves as a tree
    /// of boxes, and endFrame lays the boxes out and turns them into sprites. From one frame to the next, the context
    /// only keeps the rectangles of the boxes and which box is held.
    class Context {
    public:
        /// @brief Writes with the font, whose atlas the renderer already holds.
        explicit Context(PixelFont font);

        /// @brief Starts a frame on a screen of the given size in pixels. The scale multiplies all sizes, padding and
        /// text, so that the UI keeps its size on scaled displays. Finds the box under the mouse among those of the
        /// last frame.
        void beginFrame(const Input& input, Size screen, int scale);

        /// @brief Lays out the boxes of the frame, keeps their rectangles for the next one and turns them into sprites.
        void endFrame();

        /// @brief Returns the sprites of the last finished frame in drawing order, in pixels of the screen.
        std::span<const Sprite> sprites() const;

        /// @brief Adds a box without children to the box that is open.
        Signal box(const BoxSpec& spec);

        /// @brief Adds a box to the box that is open and opens it, so that the next boxes become its children until
        /// endBox.
        Signal beginBox(const BoxSpec& spec);

        /// @brief Closes the box that beginBox opened last.
        void endBox();

        /// @brief Shows a line of text.
        void label(std::string_view text);

        /// @brief Shows a button with the visible text of the label and returns whether it was clicked in this frame.
        bool button(std::string_view label);

    private:
        /// @brief A box of the current frame and its layout, with one entry per axis in the arrays.
        struct Node {
            Id id;
            bool keyed = false;
            std::size_t parent = 0;
            std::string text;
            std::array<SizeRule, 2> rules;
            Axis childAxis = Axis::Y;
            bool clickable = false;
            std::optional<Color> background;
            std::optional<Color> textColor;
            std::array<float, 2> position{};
            std::array<float, 2> size{};

            /// @brief Where the next child starts along the child axis, measured from the position.
            float nextChild = 0.0f;

            Rect rect() const {
                return {
                    .position = {position[0], position[1]},
                    .size = {size[0], size[1]},
                };
            }
        };

        /// @brief Computes the size and position of every node.
        void layout();

        /// @brief Turns the nodes into sprites, parents before their children.
        void draw();

        PixelFont font;
        const Input* frameInput = nullptr;
        int frameScale = 1;

        /// @brief The clickable box under the mouse that may react in this frame, which its signal calls hovered.
        Id hot{};

        /// @brief The box on which the left button went down and that keeps the mouse until the button comes up,
        /// which its signal calls held.
        Id active{};

        /// @brief The boxes of the frame in the order of building, so every parent comes before its children.
        std::vector<Node> nodes;

        /// @brief The indices of the boxes that beginBox opened and endBox has not closed yet, the root first.
        std::vector<std::size_t> openNodes;

        /// @brief The rectangles of the last frame, for every box with a label.
        std::unordered_map<Id, Rect> lastRects;

        /// @brief The clickable boxes of the last frame in drawing order, so the last one under the mouse is on top.
        std::vector<Id> lastClickable;

        std::vector<Sprite> drawList;
    };
}
