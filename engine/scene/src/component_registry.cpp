#include <etude/scene/component_registry.h>

#include <etude/scene/components/camera.h>
#include <etude/scene/components/name.h>
#include <etude/scene/components/sprite_renderer.h>
#include <etude/scene/components/transform2d.h>
#include <etude/scene/components/velocity.h>

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
        registry.add<Velocity>();
    }
}
