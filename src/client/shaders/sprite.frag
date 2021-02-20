#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec2 in_TexCoord;
layout (location = 1) in vec4 in_Color;
layout (location = 2) flat in int in_TextureIndex;

layout (location = 0) out vec4 out_Color;

layout (set = 0, binding = 1) uniform sampler2D[] textures;

void main() 
{
    out_Color = texture(textures[nonuniformEXT(in_TextureIndex)], in_TexCoord) * in_Color;
}