#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(push_constant) uniform constants
{
    uint materialIndex;
} pushConstants;

struct Material {
    uvec4 textureIndices;
};

layout(set = 1, binding = 1) uniform sampler sharedSampler;
layout(set = 1, binding = 2) uniform texture2D textures[];
layout(set = 1, binding = 0) readonly buffer MaterialData
{
    Material materials[];
} materialBuffer;

layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
    Material material = materialBuffer.materials[pushConstants.materialIndex];

    const uint textureIdx = nonuniformEXT(material.textureIndices.r);

    vec4 color = texture(sampler2D(textures[textureIdx], sharedSampler), fragTexCoord).rgba;

    const float alphaThreshold = 0.5f;
    if(color.a < alphaThreshold)
    {
        discard;
    }

    outColor = color;
}