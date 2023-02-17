#version 460 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texc;

layout(location = 0) out vec2 tc;

uniform vec2 position;
uniform float size;
uniform float rotation=0;
vec2 rotate(vec2 pos,float rott){
    return mat4(cos(rott),sin(rott),-sin(rott),cos(rott))*pos;
}
void main(){
    vec2 poss = vec2(pos*size+position);
    gl_Position = vec4(poss.x/960.0-1.0,-poss.y/540.0+1.0,0.0,1.0);
    tc = rotate(texc,rotation);
}
