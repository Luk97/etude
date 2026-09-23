#include<print>

import etude.core;

int main() {
    const etude::Vec2 v{3.0f, 4.0f};
    std::println("Length of ({}, {}): {}", v.x, v.y, v.length());
    return 0;
}
