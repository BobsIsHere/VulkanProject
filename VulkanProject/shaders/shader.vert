#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

struct Vertex 
{
    vec4 pos;
    vec4 color;
    vec4 normal;
    vec2 texCoord;
};

layout(set = 0, binding = 1, std430) readonly buffer VertexData {
    Vertex vertices[];
} vertexBuffer;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec4 fragWorldNormal;

void main() 
{
    Vertex vertex = vertexBuffer.vertices[gl_VertexIndex];
    gl_Position = ubo.proj * ubo.view * ubo.model * vertex.pos;

    fragColor = vertex.color;
    fragTexCoord = vertex.texCoord;
    fragWorldNormal = vec4(mat3(ubo.model) * vertex.normal.xyz, 0.0);
}