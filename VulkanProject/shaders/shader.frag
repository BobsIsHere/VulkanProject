#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 1) in vec2 fragTexCoord;

layout(push_constant) uniform PushConstants
{
    uint materialIndex;
} pushConstants;

layout(set = 1, binding = 1) uniform sampler sharedSampler;
layout(set = 1, binding = 2) uniform texture2D textures[];

struct Material 
{
    uvec4 textureIndices;
};

layout(set = 1, binding = 0) readonly buffer MaterialData
{
    Material materials[];
} materialBuffer;

void main() 
{
    Material material = materialBuffer.materials[pushConstants.materialIndex];

    const uint textureIndex = nonuniformEXT(material.textureIndices.r);
    vec4 color = texture(
        sampler2D(textures[textureIndex], sharedSampler),
        fragTexCoord).rgba;
}