#pragma once

#include <etude/math/size.h>
#include <etude/platform/input.h>
#include <etude/render2d/pixel_font.h>
#include <etude/rendering/draw_batch.h>
#include <etude/rendering/sprite.h>
#include <etude/ui/content.h>
#include <etude/ui/interaction.h>
#include <etude/ui/modifier.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

namespace etude::ui {

    // The parts of a context and the arrangement of a layout, which only the sources of etude.ui define.
    enum class Arrangement;
    struct FrameMemory;
    class NodeTree;
    class Painter;
    class Pointer;

    /// @brief Builds the UI anew in every frame. Between beginFrame and endFrame, the building blocks box, row, column
    /// and text and the widgets made of them describe the UI as a tree of elements, and endFrame lays the elements out
    /// and turns them into sprites.
    ///
    /// This is an immediate-mode UI: the game calls a widget such as button in every frame in which it should show, and
    /// the widget answers right away, for example whether it was clicked. No widget lives on as an object. Since the
    /// elements of a frame only get their rectangles at its end, the mouse is tested against the rectangles of the last
    /// frame, so an element that moves is hit where it was one frame earlier, which the eye does not notice.
    ///
    /// Between frames, the context only keeps what the elements cannot describe anew: the picture of the last frame,
    /// which element the mouse holds, where the floating elements are and in which order, how far the elements are
    /// scrolled, and which element has the focus with its text cursor. The values themselves, such as whether a
    /// checkbox is on, stay with the game, which hands them in by reference. The parts of the context only appear here
    /// by name, so that this header shows nothing but what games and widgets use.
    class Context {
    public:
        /// @brief Writes with the font, whose atlas the renderer already holds.
        explicit Context(PixelFont font);
        ~Context();

        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        /// @brief Starts a frame on a screen of the given size in pixels. The scale multiplies all lengths and the
        /// text, so that the UI keeps its size on scaled displays. Finds what the mouse is over in the picture of the
        /// last frame, brings a pressed floating element to the front, takes the focus from an element when the press
        /// lands elsewhere and lets the wheel scroll the element under the mouse.
        void beginFrame(const Input& input, Size screen, int scale);

        /// @brief Lays out the elements of the frame and turns them into the picture that the next frame tests the
        /// mouse against. Enter and Escape take the focus away only here, at the end of the frame, so that the keyboard
        /// belongs to the UI for the whole frame in which they were pressed and the game does not react to them.
        void endFrame();

        /// @brief Returns the sprites of the last finished frame in drawing order, in pixels of the screen, without
        /// their clip rectangles.
        std::span<const Sprite> sprites() const;

        /// @brief Returns the sprites of the last finished frame as batches for the renderer, in drawing order, each
        /// with the matrix of the screen and the clip rectangle of its elements.
        std::span<const DrawBatch> batches() const;

        /// @brief Returns whether the mouse belongs to the UI in this frame, which the game should then leave alone. A
        /// press of the left button belongs to where it began until the button comes up again, to the UI if the mouse
        /// was over an element that the UI drew in the last frame. Without a press, the mouse belongs to the UI while
        /// it is over such an element.
        bool wantsMouse() const;

        /// @brief Returns whether the keyboard belongs to the UI, which is the case while an element has the focus.
        bool wantsKeyboard() const;

        /// @brief Returns the input of the frame, for widgets that follow the mouse or read the keyboard themselves.
        const Input& input() const;

        /// @brief Returns where the next typed character goes in the text of the element with the focus, in bytes. When
        /// an element gains the focus, the cursor jumps past the end of any text, so a widget that limits it to its
        /// text starts typing at the end.
        std::size_t& textCursor();

    private:
        friend Interaction box(Context& ui, const Modifier& modifier, Content content);
        friend Interaction row(Context& ui, const Modifier& modifier, Content content);
        friend Interaction column(Context& ui, const Modifier& modifier, Content content);
        friend Interaction text(Context& ui, std::string_view line, const Modifier& modifier);

        /// @brief Adds an element to the element that is open and opens it, so that the next elements become its
        /// children until closeElement. The modifier describes the element, the arrangement says how its children line
        /// up, and a line makes it a text. Returns what the mouse and the keyboard do with the element in this frame.
        Interaction openElement(
            const Modifier& modifier,
            Arrangement arrangement,
            std::optional<std::string_view> line
        );

        /// @brief Closes the element that openElement opened last.
        void closeElement();

        /// @brief The elements of the frame as a tree.
        std::unique_ptr<NodeTree> tree;

        /// @brief Follows the mouse over the picture of the last frame.
        std::unique_ptr<Pointer> pointer;

        /// @brief Draws the elements and keeps the picture until the next frame.
        std::unique_ptr<Painter> painter;

        /// @brief What the elements keep from one frame to the next.
        std::unique_ptr<FrameMemory> memory;

        /// @brief The input of the frame, from beginFrame on.
        const Input* frameInput = nullptr;

        /// @brief The scale of the frame, from beginFrame on.
        int frameScale = 1;
    };
}
