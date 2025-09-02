#version 450

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

struct Vertex 
{
    vec3 pos;
    vec3 normal;
    vec2 uv;
};

layout(std430, binding = 1) readonly buffer VertexData 
{
    Vertex vertices[];
} vertexBuffer;

layout(location = 0) out vec2 fragTexCoord;

void main() 
{
    Vertex v = vertexBuffer.vertices[gl_VertexIndex];
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(v.pos, 1.0f);

    fragTexCoord = v.uv;
}