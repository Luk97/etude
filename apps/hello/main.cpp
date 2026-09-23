#include <print>

#include <etude/core/vec2.h>

int main() {
    const etude::Vec2 v{3.0f, 4.0f};
    std::println("Length of ({}, {}): {}", v.x, v.y, v.length());
    return 0;
}
