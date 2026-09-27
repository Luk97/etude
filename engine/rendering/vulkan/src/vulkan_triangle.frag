#version 450
#extension GL_EXT_nonuniform_qualifier : require

// The texture table: one sampler for all textures, then the array of textures, which may have gaps.
layout(set = 0, binding = 0) uniform sampler nearest;
layout(set = 0, binding = 1) uniform texture2D textures[];

// Color and texture coordinates arrive interpolated between the corners, the texture index is the same everywhere.
layout(location = 0) in vec3 color;
layout(location = 1) in vec2 uv;
layout(location = 2) flat in uint textureIndex;

layout(location = 0) out vec4 fragmentColor;

void main() {
    fragmentColor = vec4(color, 1.0) * texture(sampler2D(textures[textureIndex], nearest), uv);
}
