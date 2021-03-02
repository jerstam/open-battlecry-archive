#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec2 in_TexCoord;
layout (location = 1) flat in int in_TextureIndex;

layout (location = 0) out vec4 out_Color;

layout (set = 0, binding = 1) uniform texture2D Textures[];
layout (set = 1, binding = 3) uniform sampler NearestClampSampler;

// TODO: Add color buffer

void main() 
{
    out_Color = texture(nonuniformEXT(sampler2D(Textures[in_TextureIndex], NearestClampSampler)), in_TexCoord);
}