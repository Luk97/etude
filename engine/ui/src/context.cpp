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

        /// @brief Returns the part of the rectangle inside the clip rectangle, or all of it without one.
        Rect visiblePart(Rect rect, const std::optional<Rect>& clip) {
            return clip ? clip->intersection(rect) : rect;
        }
    }

    Scope::Scope(Context& context, int boxes) : context(context), boxes(boxes) {}

    Scope::~Scope() {
        for (int i = 0; i < boxes; ++i) {
            context.endBox();
        }
    }

    Context::Context(PixelFont font) : font(font) {}

    void Context::beginFrame(const Input& input, Size screen, int scale) {
        frameInput = &input;
        frameScale = scale;
        const Rect screenArea{
            .size = {static_cast<float>(screen.width), static_cast<float>(screen.height)},
        };
        projection = Mat3::orthographic(screenArea);
        const Vec2 mouse = input.mousePosition();

        // The panel in front under the mouse covers everything below it, so only its own boxes can become hot.
        topPanel = Id{};
        for (auto id = panelOrder.rbegin(); id != panelOrder.rend(); ++id) {
            if (const auto rect = lastRects.find(*id); rect != lastRects.end() && rect->second.contains(mouse)) {
                topPanel = *id;
                break;
            }
        }

        // The box drawn last lies on top. While a box is held, no other box can become hot.
        hot = Id{};
        for (auto clickable = lastClickable.rbegin(); clickable != lastClickable.rend(); ++clickable) {
            if (clickable->panel == topPanel && clickable->visible.contains(mouse)) {
                hot = clickable->id;
                break;
            }
        }
        if (active != Id{} && hot != active) {
            hot = Id{};
        }

        // A press on a panel brings it to the front, and a press anywhere but on the field with the focus ends the
        // typing into it.
        if (input.pressed(MouseButton::Left)) {
            if (topPanel != Id{}) {
                std::erase(panelOrder, topPanel);
                panelOrder.push_back(topPanel);
            }
            if (hot != focus) {
                focus = Id{};
            }
        }

        // The root keeps the size of the screen. Its rules only say that it does not fit its children.
        nodes.clear();
        framePanels.clear();
        nodes.push_back({
            .id = rootId,
            .rules = {SizeRule::pixels(0.0f), SizeRule::pixels(0.0f)},
            .size = {static_cast<float>(screen.width), static_cast<float>(screen.height)},
        });
        openNodes.assign(1, 0);
    }

    void Context::endFrame() {
        ETUDE_ASSERT(openNodes.size() == 1);
        nodes.front().end = nodes.size();
        layout();

        // The length of the content decides how far a panel can scroll.
        for (const FramePanel& panel : framePanels) {
            const Node& content = nodes[panel.content];
            PanelState& state = panels.at(panel.id);
            state.maxScroll = std::max(content.nextChild + 2.0f * content.padding - content.size[1], 0.0f);
            state.scroll = std::min(state.scroll, state.maxScroll);
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

    std::span<const DrawBatch> Context::batches() const {
        return drawBatches;
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

        ++nodes[parent].childCount;
        nodes.push_back({
            .id = id,
            .keyed = keyed,
            .parent = parent,
            .text = std::string(spec.text.value_or(visibleText(spec.label))),
            .rules = {spec.width, spec.height},
            .childAxis = spec.childAxis,
            .padding = spec.padding * frameScale,
            .gap = spec.gap * frameScale,
            .clip = spec.clip,
            .clickable = spec.clickable,
            .background = spec.background,
            .textColor = spec.textColor,
            .panel = nodes[parent].panel,
        });
        openNodes.push_back(nodes.size() - 1);
        return signal;
    }

    void Context::endBox() {
        ETUDE_ASSERT(openNodes.size() > 1);
        nodes[openNodes.back()].end = nodes.size();
        openNodes.pop_back();
    }

    void Context::layout() {
        for (const Axis axis : {Axis::X, Axis::Y}) {
            const std::size_t a = indexOf(axis);

            // Sizes that need nothing but the node itself. A box that fits its children starts with its padding and
            // the gaps between them. The root at index 0 keeps the size of the screen.
            for (std::size_t i = 1; i < nodes.size(); ++i) {
                Node& node = nodes[i];
                const SizeRule rule = node.rules[a];
                if (rule.kind == SizeRule::Kind::Pixels) {
                    node.size[a] = rule.value * frameScale;
                } else if (rule.kind == SizeRule::Kind::Text) {
                    const Vec2 textSize = PixelFont::measure(node.text, frameScale);
                    node.size[a] = (axis == Axis::X ? textSize.x : textSize.y) + 2.0f * rule.value * frameScale;
                } else if (rule.kind == SizeRule::Kind::Children) {
                    const std::size_t gaps = node.childAxis == axis && node.childCount > 1 ? node.childCount - 1 : 0;
                    node.size[a] = 2.0f * node.padding + static_cast<float>(gaps) * node.gap;
                }
            }

            // Shares of the space inside the parent, which comes earlier in the list and so has its size already.
            for (std::size_t i = 1; i < nodes.size(); ++i) {
                Node& node = nodes[i];
                if (node.rules[a].kind == SizeRule::Kind::ParentShare) {
                    const Node& parent = nodes[node.parent];
                    node.size[a] = (parent.size[a] - 2.0f * parent.padding) * node.rules[a].value;
                }
            }

            // Parents that fit their children. Walking backwards meets all children of a node before the node itself.
            for (std::size_t i = nodes.size() - 1; i > 0; --i) {
                const Node& child = nodes[i];
                Node& parent = nodes[child.parent];
                if (child.floating || parent.rules[a].kind != SizeRule::Kind::Children) {
                    continue;
                }
                parent.size[a] = parent.childAxis == axis
                                     ? parent.size[a] + child.size[a]
                                     : std::max(parent.size[a], child.size[a] + 2.0f * parent.padding);
            }
        }

        // Every child starts inside the padding of its parent, where its previous sibling ends plus the gap, moved back
        // by the scrolling. A floating child sits at its offset instead.
        for (std::size_t i = 1; i < nodes.size(); ++i) {
            Node& node = nodes[i];
            Node& parent = nodes[node.parent];
            if (node.floating) {
                node.position = {parent.position[0] + node.offset.x, parent.position[1] + node.offset.y};
                continue;
            }
            const std::size_t along = indexOf(parent.childAxis);
            const std::size_t across = 1 - along;
            if (parent.placedChildren > 0) {
                parent.nextChild += parent.gap;
            }
            node.position[along] = parent.position[along] + parent.padding + parent.nextChild - parent.scroll;
            node.position[across] = parent.position[across] + parent.padding;
            parent.nextChild += node.size[along];
            ++parent.placedChildren;
        }
    }

    void Context::draw() {
        drawList.clear();
        ranges.clear();
        lastRects.clear();
        lastClickable.clear();

        drawNodes(0, nodes.size(), true);
        for (const Id id : panelOrder) {
            if (const auto panel = std::ranges::find(framePanels, id, &FramePanel::id); panel != framePanels.end()) {
                drawNodes(panel->node, nodes[panel->node].end, false);
            }
        }

        drawBatches.clear();
        for (const BatchRange& range : ranges) {
            drawBatches.push_back({
                .viewProjection = projection,
                .clip = range.clip,
                .sprites = std::span<const Sprite>(drawList).subspan(range.first, range.count),
            });
        }
    }

    void Context::drawNodes(std::size_t first, std::size_t last, bool skipPanels) {
        for (std::size_t i = first; i < last;) {
            if (skipPanels && nodes[i].floating) {
                i = nodes[i].end;
                continue;
            }
            drawNode(i);
            ++i;
        }
    }

    void Context::drawNode(std::size_t index) {
        Node& node = nodes[index];
        const Rect rect = node.rect();

        // The root is its own parent, and it clips nothing.
        const std::optional<Rect> clip = nodes[node.parent].childClip;
        node.childClip = node.clip ? std::optional<Rect>(visiblePart(rect, clip)) : clip;

        if (node.keyed) {
            lastRects[node.id] = rect;
            if (node.clickable) {
                lastClickable.push_back({
                    .id = node.id,
                    .visible = visiblePart(rect, clip),
                    .panel = node.panel,
                });
            }
        }

        const std::size_t first = drawList.size();
        if (node.background) {
            const bool hovered = node.clickable && node.id == hot;
            const bool held = node.clickable && node.id == active;
            drawList.push_back({
                .position = rect.position,
                .size = rect.size,
                .color = highlight(*node.background, hovered, held),
            });
        }
        if (node.textColor) {
            const Vec2 textSize = PixelFont::measure(node.text, frameScale);
            const Vec2 textPosition = rect.position + (rect.size - textSize) * 0.5f;
            font.appendText(drawList, node.text, textPosition, frameScale, *node.textColor);
        }

        // Sprites that follow each other with the same clip rectangle share a batch.
        if (const std::size_t count = drawList.size() - first; count > 0) {
            if (ranges.empty() || ranges.back().clip != clip) {
                ranges.push_back({
                    .clip = clip,
                    .first = first,
                });
            }
            ranges.back().count += count;
        }
    }

    Color Context::highlight(Color color, bool hovered, bool held) {
        if (held) {
            return mix(color, black, 0.25f);
        }
        if (hovered) {
            return mix(color, white, 0.15f);
        }
        return color;
    }
}
