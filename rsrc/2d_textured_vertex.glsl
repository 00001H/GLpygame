#version 460 core

layout(location = 0) in vec2 pos;

layout(location = 0) out vec2 texc;

uniform vec2 position;
uniform vec2 imgdims;
uniform vec2 rotation_center=vec2(0.5,0.5);
uniform float rotation=0.0;
uniform float hsw=960.0;
uniform float hsh=540.0;
uniform bool flipv=true;
uniform vec4 region=vec4(0.0,0.0,1.0,1.0);
vec2 rotate(vec2 pos,float rott){
    return mat2(cos(rott),sin(rott),-sin(rott),cos(rott))*pos;
}
void main(){
    if(rotation>0.00001){
        vec2 centered = pos-rotation_center;
        vec2 rrmc = rotate(centered*imgdims,rotation)+rotation_center*imgdims;
        vec2 poss = rrmc+position;
        gl_Position = vec4(poss.x/hsw-1.0,-poss.y/hsh+1.0,0.0,1.0);
    }else{
        //Performance save: avoid redundant calculations if not rotating
        vec2 poss = pos*imgdims+position;
        gl_Position = vec4(poss.x/hsw-1.0,-poss.y/hsh+1.0,0.0,1.0);
    }
    vec2 ftc = (flipv?pos:vec2(pos.x,1.0-pos.y));
    texc = ftc*(region.zw-region.xy)+region.xy;
}
