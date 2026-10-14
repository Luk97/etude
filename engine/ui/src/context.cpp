#include <etude/ui/context.h>

#include "arrangement.h"
#include "frame_memory.h"
#include "layout.h"
#include "node_tree.h"
#include "painter.h"
#include "pointer.h"
#include "size_rule.h"

#include <etude/core/assert.h>
#include <etude/math/vec2.h>
#include <etude/ui/theme.h>

#include <algorithm>
#include <array>
#include <limits>
#include <span>
#include <string>
#include <utility>

namespace etude::ui {

    namespace {

        /// @brief How far one notch of the wheel scrolls, in pixels at scale 1.
        constexpr float scrollStep = 24.0f;
    }

    Context::Context(PixelFont font)
        : tree(std::make_unique<NodeTree>()), pointer(std::make_unique<Pointer>()),
          painter(std::make_unique<Painter>(font)), memory(std::make_unique<FrameMemory>()) {}

    Context::~Context() = default;

    void Context::beginFrame(const Input& input, Size screen, int scale) {
        // The building blocks read the input and the scale up to endFrame. The mouse is tested against the picture of
        // the last frame, which the painter still holds.
        frameInput = &input;
        frameScale = scale;
        pointer->beginFrame(input, *painter, memory->layerOrder);

        // A press on a floating element brings it to the front, and a press anywhere but on the element with the focus
        // ends the typing into it.
        if (input.pressed(MouseButton::Left)) {
            if (const Id layer = pointer->topLayer(); layer != Id{}) {
                std::erase(memory->layerOrder, layer);
                memory->layerOrder.push_back(layer);
            }
            if (pointer->hot() != memory->focus) {
                memory->focus = Id{};
            }
        }

        // The wheel scrolls the element under the mouse by one step per notch, as far as the last frame allows.
        if (const Id target = pointer->scrollTarget(); target != Id{}) {
            ScrollState& scroll = memory->scrolls[target];
            const float scrolled = scroll.offset - input.wheel() * scrollStep * static_cast<float>(scale);
            scroll.offset = std::clamp(scrolled, 0.0f, scroll.limit);
        }

        // Only now the tree of this frame starts, with nothing but the root.
        tree->beginFrame(screen);
    }

    void Context::endFrame() {
        tree->endFrame();

        // Floating elements sit where the memory keeps them, so a drag handle moves them in the same frame. Then the
        // layout pass finds where everything goes.
        for (const std::size_t index : tree->floatingNodes()) {
            Node& node = tree->node(index);
            node.offset = memory->layers.at(node.id).position;
        }
        layout(tree->nodes(), frameScale);

        // How far the children reach beyond the bottom decides how far an element can scroll. If the content has shrunk
        // below the offset, the offset shrinks with it, and the children move down by the difference in this frame.
        const std::span<Node> nodes = tree->nodes();
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            const Node& node = nodes[i];
            if (!node.scrolls) {
                continue;
            }
            ScrollState& scroll = memory->scrolls[node.id];
            scroll.limit = std::max(node.contentSize[1] + 2.0f * node.padding[1] - node.size[1], 0.0f);
            if (const float excess = scroll.offset - scroll.limit; excess > 0.0f) {
                scroll.offset = scroll.limit;
                for (std::size_t child = i + 1; child < node.end; ++child) {
                    nodes[child].position[1] += excess;
                }
            }
        }

        // Enter and Escape end the typing only now, so that the game does not see them while the element has the focus.
        // An element that this frame did not build loses the focus as well, so that a hidden field gives the keyboard
        // back to the game.
        const bool focusBuilt = std::ranges::any_of(tree->nodes(), [&](const Node& node) {
            return node.keyed && node.id == memory->focus;
        });
        if (frameInput->pressed(Key::Enter) || frameInput->pressed(Key::Escape) || !focusBuilt) {
            memory->focus = Id{};
        }

        // A released element is let go, and the painter turns the frame into the picture that the next frame tests the
        // mouse against.
        pointer->endFrame(*frameInput);
        painter->paint({
            .nodes = tree->nodes(),
            .floating = tree->floatingNodes(),
            .layerOrder = memory->layerOrder,
            .hot = pointer->hot(),
            .active = pointer->active(),
            .scale = frameScale,
        });
    }

    std::span<const Sprite> Context::sprites() const {
        return painter->sprites();
    }

    std::span<const DrawBatch> Context::batches() const {
        return painter->batches();
    }

    bool Context::wantsMouse() const {
        return pointer->wantsMouse();
    }

    bool Context::wantsKeyboard() const {
        return memory->focus != Id{};
    }

    const Input& Context::input() const {
        return *frameInput;
    }

    std::size_t& Context::textCursor() {
        return memory->cursor;
    }

    Interaction Context::openElement(
        const Modifier& modifier,
        Arrangement arrangement,
        std::optional<std::string_view> line
    ) {
        // Focusable elements and drag handles take the mouse just like clickable ones.
        const Modifier::Settings& settings = modifier.settings;
        const bool clickable = settings.clickable || settings.focusable || settings.dragHandle;

        // Elements that keep something between frames need a key, and floating elements only open at the top level.
        ETUDE_ASSERT(!settings.key.empty() || !(clickable || settings.verticalScroll || settings.floating));
        ETUDE_ASSERT(!settings.floating || tree->current() == 0);

        // Whatever the modifier leaves unset, the building block decides: a text keeps the padding of a line and is as
        // large as its text, a layout fits its children. The node holds all lengths in pixels of the screen, so the
        // scale multiplies fixed sizes, padding and gaps right here.
        const float scale = static_cast<float>(frameScale);
        const std::array<float, 2> defaultPadding =
            line ? std::array{theme::linePadding, theme::linePadding} : std::array{0.0f, 0.0f};
        const std::array<float, 2> padding = settings.padding.value_or(defaultPadding);
        std::array<SizeRule, 2> rules;
        for (std::size_t axis = 0; axis < rules.size(); ++axis) {
            if (settings.pixels[axis]) {
                rules[axis] = SizeRule::pixels(*settings.pixels[axis] * scale);
            } else if (settings.shares[axis]) {
                rules[axis] = SizeRule::share(*settings.shares[axis]);
            } else {
                rules[axis] = line ? SizeRule::text() : SizeRule::children();
            }
        }

        // The tree names the node by its key and opens it, so that the next elements become its children.
        Node node{
            .text = std::string(line.value_or(std::string_view{})),
            .rules = rules,
            .arrangement = arrangement,
            .alignment = settings.alignment,
            .padding = {padding[0] * scale, padding[1] * scale},
            .spacing = settings.spacing * scale,
            .clip = settings.clip,
            .clickable = clickable,
            .scrolls = settings.verticalScroll,
            .background = settings.background,
            .textColor = line ? std::optional(theme::textColor) : std::nullopt,
            .floating = settings.floating.has_value(),
        };
        const Id id = tree->open(settings.key, std::move(node));
        Node& opened = tree->node(tree->current());

        // A floating element starts at its position in its first frame and comes to the front. Later frames keep
        // whatever the memory says.
        if (settings.floating) {
            const auto [layer, created] = memory->layers.try_emplace(id);
            if (created) {
                layer->second.position = *settings.floating * scale;
                memory->layerOrder.push_back(id);
            }
        }

        // A scrolling element moves its children by the offset that the memory keeps.
        if (settings.verticalScroll) {
            opened.scroll = memory->scrolls[id].offset;
        }

        // Only elements that take the mouse ask the pointer, and only elements with a key have a rectangle from the
        // last frame.
        Interaction interaction = clickable ? pointer->interact(id, *frameInput) : Interaction{};
        if (opened.keyed) {
            interaction.rect = painter->rectOf(id);
        }

        // A press on a focusable element gives it the focus, with the cursor past the end of any text.
        if (settings.focusable && interaction.pressed && memory->focus != id) {
            memory->focus = id;
            memory->cursor = std::numeric_limits<std::size_t>::max();
        }
        interaction.focused = settings.focusable && memory->focus == id;

        // Dragging a handle moves its floating element, which keeps the point where the mouse grabbed it. While the
        // window holds the mouse, it reports positions beyond its edges as well, so the mouse counts as if it stayed on
        // the screen, and the point that the mouse holds stays on the screen with it.
        if (settings.dragHandle && opened.layer != Id{}) {
            LayerState& layer = memory->layers.at(opened.layer);
            const Vec2 screen = tree->node(0).rect().size;
            const Vec2 reported = frameInput->mousePosition();
            const Vec2 mouse{
                .x = std::clamp(reported.x, 0.0f, std::max(screen.x - 1.0f, 0.0f)),
                .y = std::clamp(reported.y, 0.0f, std::max(screen.y - 1.0f, 0.0f)),
            };
            if (interaction.pressed) {
                layer.grab = mouse - layer.position;
            }
            if (interaction.held) {
                layer.position = mouse - layer.grab;
            }
        }
        return interaction;
    }

    void Context::closeElement() {
        tree->close();
    }
}
