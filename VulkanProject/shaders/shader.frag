#version 450

layout(constant_id = 0) const uint TEXTURE_ARRAY_SIZE = 2;
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
    outColor = texture(sampler2D(textures[pc.textureIndex], sharedSampler), fragTexCoord);
}