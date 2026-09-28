#include <etude/scene/component_registry.h>

#include <etude/scene/components.h>

namespace etude {

    const ComponentType* ComponentRegistry::find(std::string_view name) const {
        for (const ComponentType& type : componentTypes) {
            if (type.name == name) {
                return &type;
            }
        }
        return nullptr;
    }

    void addBuiltinComponents(ComponentRegistry& registry) {
        registry.add<Name>();
        registry.add<Transform2D>();
        registry.add<SpriteRenderer>();
        registry.add<Camera>();
    }
}
