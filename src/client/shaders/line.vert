#version 450

layout (location = 0) out vec4 out_Color;

struct Vertex 
{
	vec2 position;
	uint color;
};

layout(set = 0, binding = 1) readonly buffer VertexBuffer
{   
	Vertex vertices[];
};

vec4 unpack_color(uint col) 
{
    return vec4(col & 0xff, (col >> 8) & 0xff, (col >> 16) & 0xff, (col >> 24) & 0xff) / 255.f;
}

void main() 
{
    vec2 position = vertices[gl_VertexIndex].position;
	uint color = vertices[gl_VertexIndex].color;

	gl_Position = vec4((position.x * 2.0f) - 1.f, (position.y * 2.0f) - 1.f, 0.0f, 1.0f);
	out_Color = unpack_color(color);
}