#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

// A corner of the triangle. The scalar layout packs the fields without gaps, just like TriangleVertex on the CPU.
struct Vertex {
    vec2 position;
    vec2 uv;
    vec4 color;
};

layout(buffer_reference, scalar) readonly buffer Vertices {
    Vertex vertices[];
};

// The renderer pushes the address of the buffer with the corners and the index of the texture before the draw.
layout(push_constant) uniform Constants {
    Vertices corners;
    uint textureIndex;
} constants;

layout(location = 0) out vec3 color;
layout(location = 1) out vec2 uv;
layout(location = 2) flat out uint textureIndex;

void main() {
    Vertex vertex = constants.corners.vertices[gl_VertexIndex];
    gl_Position = vec4(vertex.position, 0.0, 1.0);
    color = vertex.color.rgb;
    uv = vertex.uv;
    textureIndex = constants.textureIndex;
}
