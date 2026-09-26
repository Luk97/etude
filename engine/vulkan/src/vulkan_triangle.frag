#version 450

// Arrives interpolated between the colors of the three corners.
layout(location = 0) in vec3 color;

layout(location = 0) out vec4 fragmentColor;

void main() {
    fragmentColor = vec4(color, 1.0);
}
