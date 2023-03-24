#version 460 core

layout(location = 0) in vec2 pos;

uniform vec2 position;
uniform float size;
uniform float vertrot=0.0;
vec2 rotate(vec2 pos,float rott){
    return mat2(cos(rott),sin(rott),-sin(rott),cos(rott))*pos;
}
void main(){
    vec2 poss = vec2(pos*size);
    poss = rotate(poss,vertrot)+position;
    gl_Position = vec4(poss.x/960.0-1.0,-poss.y/540.0+1.0,0.0,1.0);
}
