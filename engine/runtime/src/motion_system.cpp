#include <etude/runtime/motion_system.h>

#include <etude/scene/components/transform2d.h>
#include <etude/scene/components/velocity.h>

namespace etude {

    void moveScene(World& world, std::chrono::duration<float> time) {
        const float seconds = time.count();
        world.each<Transform2D, Velocity>([&](Entity, Transform2D& transform, const Velocity& velocity) {
            transform.position = transform.position + velocity.linear * seconds;
            transform.rotation += velocity.angular * seconds;
        });
    }
}
