#version 460 core

layout(location = 0) in vec2 pos;

uniform vec2 position;
uniform vec2 imgdims;
uniform vec2 rotation_center=vec2(0.5,0.5);
uniform float size;
uniform float rotation=0.0;
uniform float hsw=960.0;
uniform float hsh=540.0;
vec2 rotate(vec2 pos,float rott){
    return mat2(cos(rott),sin(rott),-sin(rott),cos(rott))*pos;
}
void main(){
    vec2 centered = pos-rotation_center;
    vec2 rrmc = rotate(centered*imgdims,rotation)+rotation_center*imgdims;
    vec2 poss = rrmc*size+position;
    gl_Position = vec4(poss.x/hsw-1.0,-poss.y/hsh+1.0,0.0,1.0);
}
