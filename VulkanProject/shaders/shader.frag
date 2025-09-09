#version 450

layout(constant_id = 0) const uint TEXTURE_ARRAY_SIZE = 1;
layout(push_constant) uniform constants
{
    uint textureIndex;
} pc;

layout(set = 1, binding = 0) uniform sampler sharedSampler;
layout(set = 1, binding = 1) uniform texture2D textures[TEXTURE_ARRAY_SIZE];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
    uint idx = pc.textureIndex;

    // Bounds check: if out-of-range -> fallback color
    if (idx >= TEXTURE_ARRAY_SIZE) 
    {
        outColor = vec4(1.0, 1.0, 0.0, 1.0);
        return;
    }

    vec4 color = texture(sampler2D(textures[idx], sharedSampler), fragTexCoord);
    outColor = color;
}