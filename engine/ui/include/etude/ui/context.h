#pragma once

#include <etude/math/color.h>
#include <etude/math/mat3.h>
#include <etude/math/rect.h>
#include <etude/math/size.h>
#include <etude/math/vec2.h>
#include <etude/platform/input.h>
#include <etude/render2d/pixel_font.h>
#include <etude/rendering/draw_batch.h>
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

    class Context;

    /// @brief Closes the boxes that a panel, row or column opened once it goes out of scope, so that the widgets in
    /// between become their content.
    class [[nodiscard]] Scope {
    public:
        Scope(const Scope&) = delete;
        Scope& operator=(const Scope&) = delete;
        ~Scope();

    private:
        friend class Context;

        Scope(Context& context, int boxes);

        Context& context;
        int boxes;
    };

    /// @brief Describes a panel: its title, where it first appears and how large it is, in pixels at scale 1.
    struct PanelSpec {
        /// @brief Names the panel and holds its title, like the label of a box.
        std::string_view title;

        Vec2 position;
        Vec2 size;
    };

    /// @brief Builds the UI anew in every frame. Between beginFrame and endFrame, widgets describe themselves as a tree
    /// of boxes, and endFrame lays the boxes out and turns them into sprites. From one frame to the next, the context
    /// only keeps the rectangles of the boxes, which box is held and where the panels are.
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

        /// @brief Returns the sprites of the last finished frame in drawing order, in pixels of the screen, without
        /// their clip rectangles.
        std::span<const Sprite> sprites() const;

        /// @brief Returns the sprites of the last finished frame as batches for the renderer, in drawing order, each
        /// with the matrix of the screen and the clip rectangle of its boxes.
        std::span<const DrawBatch> batches() const;

        /// @brief Adds a box without children to the box that is open.
        Signal box(const BoxSpec& spec);

        /// @brief Adds a box to the box that is open and opens it, so that the next boxes become its children until
        /// endBox.
        Signal beginBox(const BoxSpec& spec);

        /// @brief Closes the box that beginBox opened last.
        void endBox();

        /// @brief Opens a panel, a box with a title that floats above everything else. Dragging the title moves the
        /// panel, a click anywhere on it brings it to the front, and the wheel scrolls its content while the mouse is
        /// over it. The widgets up to the end of the scope become the content, cut off at the edge of the panel.
        /// Panels only open at the top level, not inside other boxes.
        Scope panel(const PanelSpec& spec);

        /// @brief Opens a row, whose widgets follow each other from left to right up to the end of the scope.
        Scope row();

        /// @brief Opens a column, whose widgets follow each other from top to bottom up to the end of the scope.
        Scope column();

        /// @brief Shows a line of text as it is.
        void label(std::string_view text);

        /// @brief Shows a button with the visible text of the label and returns whether it was clicked in this frame.
        bool button(std::string_view label);

        /// @brief Shows a square that a click turns on and off, with the visible text of the label beside it, which
        /// takes clicks as well. Returns whether the value changed in this frame.
        bool checkbox(std::string_view label, bool& value);

        /// @brief Shows a bar that sets the value between min and max to where the mouse presses or drags it, with the
        /// visible text of the label before it and the value after it. Returns whether the value changed in this frame.
        /// Min has to be smaller than max.
        bool slider(std::string_view label, float& value, float min, float max);

        /// @brief Shows a field that takes typed text from a click on it until Enter, Escape or a click elsewhere.
        /// Backspace and Delete remove characters, the arrow keys, Home and End move the cursor, and held keys repeat.
        /// Returns whether the text changed in this frame. The text has to be valid UTF-8, and the field cuts off
        /// what does not fit.
        bool textField(std::string_view label, std::string& text);

    private:
        /// @brief A box of the current frame and its layout, with one entry per axis in the arrays.
        struct Node {
            Id id;
            bool keyed = false;
            std::size_t parent = 0;
            std::string text;
            std::array<SizeRule, 2> rules;
            Axis childAxis = Axis::Y;
            float padding = 0.0f;
            float gap = 0.0f;
            bool clip = false;
            bool clickable = false;
            std::optional<Color> background;
            std::optional<Color> textColor;

            /// @brief The panel that the box belongs to, Id{} outside of panels.
            Id panel;

            /// @brief Whether the box sits at its offset from the parent instead of after its previous sibling, as
            /// panels do. It does not count towards the size of its parent either.
            bool floating = false;
            Vec2 offset;

            /// @brief How far the children are moved back along the child axis, for scrolling.
            float scroll = 0.0f;

            /// @brief How many children follow each other along the child axis, without floating ones.
            std::size_t childCount = 0;

            /// @brief One past the last node in the subtree of the box.
            std::size_t end = 0;

            std::array<float, 2> position{};
            std::array<float, 2> size{};

            /// @brief Where the next child starts along the child axis, measured from the padding, which is the length
            /// of all children and their gaps once the layout is done.
            float nextChild = 0.0f;
            std::size_t placedChildren = 0;

            /// @brief The clip rectangle that applies to the children, found while drawing.
            std::optional<Rect> childClip;

            Rect rect() const {
                return {
                    .position = {position[0], position[1]},
                    .size = {size[0], size[1]},
                };
            }
        };

        /// @brief What the context keeps about a panel from one frame to the next, in pixels.
        struct PanelState {
            Vec2 position;

            /// @brief Where the mouse grabbed the title, measured from the position.
            Vec2 grab;

            float scroll = 0.0f;

            /// @brief How far the content could scroll in the last frame.
            float maxScroll = 0.0f;
        };

        /// @brief A panel of the current frame with the nodes of the panel and of its content.
        struct FramePanel {
            Id id;
            std::size_t node = 0;
            std::size_t content = 0;
        };

        /// @brief A clickable box of the last frame with the part of it that was not cut off.
        struct Clickable {
            Id id;
            Rect visible;
            Id panel;
        };

        /// @brief Sprites in a row of the draw list that share a clip rectangle.
        struct BatchRange {
            std::optional<Rect> clip;
            std::size_t first = 0;
            std::size_t count = 0;
        };

        /// @brief Computes the size and position of every node.
        void layout();

        /// @brief Turns the nodes into sprites, first everything outside of panels, then the panels from back to front.
        void draw();

        /// @brief Draws the nodes from first to last, parents before their children, and jumps over panels if asked.
        void drawNodes(std::size_t first, std::size_t last, bool skipPanels);

        /// @brief Draws one node within the clip rectangle of its parent and keeps its rectangle for the next frame.
        void drawNode(std::size_t index);

        /// @brief Applies the typed text and the editing keys of the frame to the text of the field with the focus,
        /// and returns whether the text changed.
        bool edit(std::string& text);

        /// @brief Returns the color of a clickable box: lighter while the mouse is over it, darker while it is held.
        static Color highlight(Color color, bool hovered, bool held);

        PixelFont font;
        const Input* frameInput = nullptr;
        int frameScale = 1;

        /// @brief Maps pixels of the screen to clip space, for the batches.
        Mat3 projection;

        /// @brief The clickable box under the mouse that may react in this frame, which its signal calls hovered.
        Id hot{};

        /// @brief The box on which the left button went down and that keeps the mouse until the button comes up,
        /// which its signal calls held.
        Id active{};

        /// @brief The text field that takes the typed text.
        Id focus{};

        /// @brief Where the next typed character goes in the text of the field with the focus, in bytes.
        std::size_t cursor = 0;

        /// @brief The panel in front under the mouse, which covers everything below it.
        Id topPanel{};

        /// @brief The boxes of the frame in the order of building, so every parent comes before its children.
        std::vector<Node> nodes;

        /// @brief The indices of the boxes that beginBox opened and endBox has not closed yet, the root first.
        std::vector<std::size_t> openNodes;

        /// @brief The rectangles of the last frame, for every box with a label.
        std::unordered_map<Id, Rect> lastRects;

        /// @brief The clickable boxes of the last frame in drawing order, so the last one under the mouse is on top.
        std::vector<Clickable> lastClickable;

        std::unordered_map<Id, PanelState> panels;

        /// @brief The panels from back to front.
        std::vector<Id> panelOrder;

        std::vector<FramePanel> framePanels;
        std::vector<Sprite> drawList;
        std::vector<BatchRange> ranges;
        std::vector<DrawBatch> drawBatches;
    };
}
