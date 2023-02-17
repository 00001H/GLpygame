#version 460 core

#extension GL_ARB_bindless_texture:require

layout(location = 0) in vec2 texcoords;

out vec4 fragcolor;

layout(bindless_sampler) uniform sampler2D img;

uniform float transparency=1.0f;
uniform float brightness=1.0f;
uniform mat3 kernel;
uniform float offset = 1.0f/300.0f;

void main(){
    vec2 offsets[9] = vec2[](
        vec2(-offset, offset),// top-left
        vec2( 0.0f  , offset),// top-center
        vec2( offset, offset),// top-right
        vec2(-offset, 0.0f  ),// center-left
        vec2( 0.0f  , 0.0f  ),// center-center
        vec2( offset, 0.0f  ),// center-right
        vec2(-offset,-offset),// bottom-left
        vec2( 0.0f  ,-offset),// bottom-center
        vec2( offset,-offset) // bottom-right    
    );
    float krnl[9] = float[](
        kernel[0].x,
        kernel[0].y,
        kernel[0].z,
        kernel[1].x,
        kernel[1].y,
        kernel[1].z,
        kernel[2].x,
        kernel[2].y,
        kernel[2].z
    );
    vec4 sampleTex[9];
    for(int i=0;i<9;i++){
        sampleTex[i] = texture(img,texcoords.xy+offsets[i]);
    }
    vec4 color = vec4(0.0);
    for(int i=0;i<9;i++)
        color += vec4(sampleTex[i].rgb*krnl[i],sampleTex[i].a);
    color = vec4(color.rgb*brightness,color.a*transparency);
    fragcolor = color;
}
