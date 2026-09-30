#include <etude/ui/context.h>

#include <etude/core/assert.h>

#include <algorithm>

namespace etude::ui {

    namespace {

        constexpr Color white{
            .r = 1.0f,
            .g = 1.0f,
            .b = 1.0f,
        };

        constexpr Color black = {};

        /// @brief Returns the index of the axis in the arrays of a node.
        constexpr std::size_t indexOf(Axis axis) {
            return static_cast<std::size_t>(axis);
        }

        /// @brief Mixes two colors, from the first at 0 to the second at 1, and keeps the alpha of the first.
        Color mix(Color from, Color to, float amount) {
            return {
                .r = from.r + (to.r - from.r) * amount,
                .g = from.g + (to.g - from.g) * amount,
                .b = from.b + (to.b - from.b) * amount,
                .a = from.a,
            };
        }
    }

    Context::Context(PixelFont font) : font(font) {}

    void Context::beginFrame(const Input& input, Size screen, int scale) {
        frameInput = &input;
        frameScale = scale;

        // The box drawn last lies on top. While a box is held, no other box can become hot.
        hot = Id{};
        for (auto id = lastClickable.rbegin(); id != lastClickable.rend(); ++id) {
            if (lastRects.at(*id).contains(input.mousePosition())) {
                hot = *id;
                break;
            }
        }
        if (active != Id{} && hot != active) {
            hot = Id{};
        }

        // The root keeps the size of the screen. Its rules only say that it does not fit its children.
        nodes.clear();
        nodes.push_back({
            .id = rootId,
            .rules = {SizeRule::pixels(0.0f), SizeRule::pixels(0.0f)},
            .size = {static_cast<float>(screen.width), static_cast<float>(screen.height)},
        });
        openNodes.assign(1, 0);
    }

    void Context::endFrame() {
        ETUDE_ASSERT(openNodes.size() == 1);
        layout();

        lastRects.clear();
        lastClickable.clear();
        for (const Node& node : nodes) {
            if (node.keyed) {
                lastRects[node.id] = node.rect();
                if (node.clickable) {
                    lastClickable.push_back(node.id);
                }
            }
        }

        // Without the left button down, no box is held any more, even if the held box has disappeared.
        if (!frameInput->held(MouseButton::Left)) {
            active = Id{};
        }
        draw();
    }

    std::span<const Sprite> Context::sprites() const {
        return drawList;
    }

    Signal Context::box(const BoxSpec& spec) {
        const Signal signal = beginBox(spec);
        endBox();
        return signal;
    }

    Signal Context::beginBox(const BoxSpec& spec) {
        ETUDE_ASSERT(!spec.clickable || !spec.label.empty());
        const std::size_t parent = openNodes.back();

        // An empty label leaves the hash of the parent as it is, so the children of a box without a label are named
        // as if they belonged to its parent.
        const Id id = makeId(spec.label, nodes[parent].id);
        const bool keyed = !spec.label.empty();

        Signal signal;
        if (const auto rect = lastRects.find(id); keyed && rect != lastRects.end()) {
            signal.rect = rect->second;
        }
        if (spec.clickable) {
            signal.hovered = hot == id;
            if (signal.hovered && frameInput->pressed(MouseButton::Left)) {
                active = id;
                signal.pressed = true;
            }
            signal.held = active == id && frameInput->held(MouseButton::Left);
            signal.clicked = active == id && signal.hovered && frameInput->released(MouseButton::Left);
        }

        nodes.push_back({
            .id = id,
            .keyed = keyed,
            .parent = parent,
            .text = std::string(visibleText(spec.label)),
            .rules = {spec.width, spec.height},
            .childAxis = spec.childAxis,
            .clickable = spec.clickable,
            .background = spec.background,
            .textColor = spec.text,
        });
        openNodes.push_back(nodes.size() - 1);
        return signal;
    }

    void Context::endBox() {
        ETUDE_ASSERT(openNodes.size() > 1);
        openNodes.pop_back();
    }

    void Context::layout() {
        for (const Axis axis : {Axis::X, Axis::Y}) {
            const std::size_t a = indexOf(axis);

            // Sizes that need nothing but the node itself. The root at index 0 keeps the size of the screen.
            for (std::size_t i = 1; i < nodes.size(); ++i) {
                Node& node = nodes[i];
                const SizeRule rule = node.rules[a];
                if (rule.kind == SizeRule::Kind::Pixels) {
                    node.size[a] = rule.value * frameScale;
                } else if (rule.kind == SizeRule::Kind::Text) {
                    const Vec2 textSize = PixelFont::measure(node.text, frameScale);
                    node.size[a] = (axis == Axis::X ? textSize.x : textSize.y) + 2.0f * rule.value * frameScale;
                }
            }

            // Shares of the parent, which comes earlier in the list and so has its size already.
            for (std::size_t i = 1; i < nodes.size(); ++i) {
                Node& node = nodes[i];
                if (node.rules[a].kind == SizeRule::Kind::ParentShare) {
                    node.size[a] = nodes[node.parent].size[a] * node.rules[a].value;
                }
            }

            // Parents that fit their children. Walking backwards meets all children of a node before the node itself.
            for (std::size_t i = nodes.size() - 1; i > 0; --i) {
                const Node& child = nodes[i];
                Node& parent = nodes[child.parent];
                if (parent.rules[a].kind == SizeRule::Kind::Children) {
                    parent.size[a] = parent.childAxis == axis ? parent.size[a] + child.size[a]
                                                              : std::max(parent.size[a], child.size[a]);
                }
            }
        }

        // Every child starts where its previous sibling ends, along the child axis of the parent.
        for (std::size_t i = 1; i < nodes.size(); ++i) {
            Node& node = nodes[i];
            Node& parent = nodes[node.parent];
            const std::size_t along = indexOf(parent.childAxis);
            node.position = parent.position;
            node.position[along] += parent.nextChild;
            parent.nextChild += node.size[along];
        }
    }

    void Context::draw() {
        drawList.clear();
        for (const Node& node : nodes) {
            const Rect rect = node.rect();
            if (node.background) {
                Color color = *node.background;
                if (node.clickable && node.id == active) {
                    color = mix(color, black, 0.25f);
                } else if (node.clickable && node.id == hot) {
                    color = mix(color, white, 0.15f);
                }
                drawList.push_back({
                    .position = rect.position,
                    .size = rect.size,
                    .color = color,
                });
            }
            if (node.textColor) {
                const Vec2 textSize = PixelFont::measure(node.text, frameScale);
                const Vec2 textPosition = rect.position + (rect.size - textSize) * 0.5f;
                font.appendText(drawList, node.text, textPosition, frameScale, *node.textColor);
            }
        }
    }
}
