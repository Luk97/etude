#version 450
#extension GL_EXT_nonuniform_qualifier : require

// The texture table: one sampler for all textures, then the array of textures, which may have gaps.
layout(set = 0, binding = 0) uniform sampler nearest;
layout(set = 0, binding = 1) uniform texture2D textures[];

layout(location = 0) in vec2 uv;
layout(location = 1) flat in uint textureIndex;

layout(location = 0) out vec4 fragmentColor;

void main() {
    // Sprites of one draw use different textures, so neighboring fragments may pick different entries of the table.
    // nonuniformEXT tells the GPU, otherwise it may read the entry of just one of them.
    fragmentColor = texture(sampler2D(textures[nonuniformEXT(textureIndex)], nearest), uv);
}
