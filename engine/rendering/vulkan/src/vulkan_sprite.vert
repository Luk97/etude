#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

// A sprite as the renderer copies it from the CPU. The scalar layout packs the fields without gaps, just like Sprite.
struct Sprite {
    vec2 position;
    vec2 size;
    float rotation;
    uint textureIndex;
};

layout(buffer_reference, scalar) readonly buffer Sprites {
    Sprite sprites[];
};

// The renderer pushes the address of the sprites of the frame and the matrix from world coordinates to clip space.
layout(push_constant, scalar) uniform Constants {
    Sprites instances;
    mat3 viewProjection;
} constants;

// Two triangles cover the unit square. Scaled by the size of a sprite, they span its rectangle.
const vec2 corners[6] = vec2[](
    vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0),
    vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(1.0, 1.0)
);

layout(location = 0) out vec2 uv;
layout(location = 1) flat out uint textureIndex;

void main() {
    Sprite sprite = constants.instances.sprites[gl_InstanceIndex];
    vec2 corner = corners[gl_VertexIndex];

    // Turns the corner around the middle of the sprite like Mat3::rotation, clockwise because y points down.
    float cosine = cos(sprite.rotation);
    float sine = sin(sprite.rotation);
    vec2 offset = mat2(cosine, sine, -sine, cosine) * ((corner - 0.5) * sprite.size);
    vec3 clip = constants.viewProjection * vec3(sprite.position + 0.5 * sprite.size + offset, 1.0);
    gl_Position = vec4(clip.xy, 0.0, 1.0);
    uv = corner;
    textureIndex = sprite.textureIndex;
}
