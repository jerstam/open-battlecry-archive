#version 450

layout (location = 0) out vec2 out_TexCoord;
layout (location = 1) out vec4 out_Color;
layout (location = 2) flat out int out_TextureIndex;

struct Instance
{
	vec2 position;
	float scale;
    uint color;
	int textureIndex;
}; 

layout(std430, set = 0, binding = 0) readonly buffer InstanceBuffer
{   
	Instance instances[];
};

layout (std430, push_constant) uniform Constants
{
    vec2 offset;
    vec2 size;
} constants;

const vec4 vertices[4] = vec4[4](
    vec4(-0.5f, -0.5f, 0.0f, 0.0f),
    vec4(-0.5f,  0.5f, 0.0f, 1.0f),
    vec4( 0.5f,  0.5f, 1.0f, 1.0f),
    vec4( 0.5f, -0.5f, 1.0f, 0.0f)
);

vec4 unpack_color(uint col) 
{
    return vec4(col & 0xff, (col >> 8) & 0xff, (col >> 16) & 0xff, (col >> 24) & 0xff) / 255.f;
}

void main() 
{
	uint index = gl_VertexIndex;
	uint instanceIndex = gl_VertexIndex / 4;

	//float x = float(index & uint(1)) - 0.5f;
	//float y = float(index / uint(2)) - 0.5f;
	vec4 vertex = vertices[index];
	vec2 instancePosition = instances[instanceIndex].position;
    float instanceScale = instances[instanceIndex].scale;
    uint instanceColor = instances[instanceIndex].color;

	vec2 position = vertex.xy * instanceScale * constants.size + instancePosition * constants.size + constants.offset;
	gl_Position = vec4(position, 0.0f, 1.0f);
	out_TexCoord = vertex.zw;
    out_Color = unpack_color(instanceColor);
	out_TextureIndex = instances[instanceIndex].textureIndex;
}