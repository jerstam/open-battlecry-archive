#version 450

layout (location = 0) out vec2 out_TexCoord;
layout (location = 1) flat out int out_TextureIndex;

struct Sprite
{
	vec2 position;
	float scale;
	int textureIndex;
};   
 
layout(std430, set = 0, binding = 0) readonly buffer SpriteBuffer
{   
	Sprite sprites[];
}; 
 
layout (std430, push_constant) uniform Constants
{
    vec2 cameraPosition;
    vec2 cameraSize;
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
	vec2 instancePosition = sprites[instanceIndex].position;
    float instanceScale = sprites[instanceIndex].scale;
    //uint instanceColor = sprites[instanceIndex].color;

	vec2 position = vertex.xy * instanceScale * constants.cameraSize + instancePosition * constants.cameraSize + constants.cameraPosition;
	gl_Position = vec4(position, 0.0f, 1.0f);
	out_TexCoord = vertex.zw;
    //out_Color = unpack_color(instanceColor);
	out_TextureIndex = sprites[instanceIndex].textureIndex;
} 