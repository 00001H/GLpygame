#version 460 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texc;

layout(location = 0) out vec2 tc;
layout(location = 1) out vec2 unrot_tc;

uniform vec2 position;
uniform float size;
uniform float rotation=0.0;
vec2 rotate(vec2 pos,float rott){
    return mat2(cos(rott),sin(rott),-sin(rott),cos(rott))*pos;
}
void main(){
    vec2 poss = vec2(pos*size+position);
    gl_Position = vec4(poss.x/960.0-1.0,-poss.y/540.0+1.0,0.0,1.0);
    tc = rotate(texc-vec2(0.5),rotation)+vec2(0.5);
    unrot_tc = texc;
}
