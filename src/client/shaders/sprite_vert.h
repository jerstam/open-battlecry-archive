#  
  g                 GLSL.std.450                      main       /   C   T   \   `        sprite.vert  b   �     #version 450

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
}      
 GL_GOOGLE_cpp_style_line_directive    GL_GOOGLE_include_directive      main         gl_VertexIndex    %   indexable     +   Sprite    +       position      +      scale     +      textureIndex      -   SpriteBuffer      -       sprites   /         A   Constants     A       cameraPosition    A      cameraSize    C   constants     R   gl_PerVertex      R       gl_Position   R      gl_PointSize      R      gl_ClipDistance   R      gl_CullDistance   T         \   out_TexCoord      `   out_TextureIndex    J entry-point main    J client vulkan100    J target-env spirv1.5 J target-env vulkan1.2    J entry-point main    G        *   G  %      H  +       #       H  +      #      H  +      #      G  ,         H  -          H  -       #       G  -      G  /   "       G  /   !       H  A       #       H  A      #      G  A      H  R              H  R            H  R            H  R            G  R      G  \          G  `      G  `              !                       
                   
   ;           +  
                                         +                      +           �+            ,                    +           ?+          �?,                    ,                     ,     !               ,     "             !      $           (           +   (      
     ,   +     -   ,      .      -   ;  .   /      +  
   0          2      (   +  
   8         9           A   (   (      B   	   A   ;  B   C   	      D   	   (   +     P        Q      P     R         Q   Q      S      R   ;  S   T         Y            [      (   ;  [   \         _      
   ;  _   `      +  
   b         c      
   6               �     ;  $   %      "        &       =  
         |                '       �  
            |                +       A     &   %      =     '   &        ,       A  2   3   /   0      0   =  (   4   3        -       A  9   :   /   0      8   =     ;   :        0       O  (   >   '   '          �  (   @   >   ;   A  D   E   C   8   =  (   F   E   = �  (   f   @   4        0       �  (   L   F   f   A  D   M   C   0   =  (   N   M   �  (   O   L   N        1       Q     V   O       Q     W   O      P     X   V   W         A  Y   Z   T   0   >  Z   X        2       O  (   ^   '   '         >  \   ^        4       A  c   d   /   0      b   =  
   e   d   >  `   e   �  8  