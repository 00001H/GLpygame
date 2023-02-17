#version 460 core
#extension GL_ARB_bindless_texture:require
layout(location = 0) in vec2 texc;

out vec4 fragcolor;//no need to specify location since only 1 output(color)

layout(bindless_sampler) uniform sampler2D tex;
uniform float bright;
uniform float alpha;

void main(){
    vec2 texcc = texc;
    vec4 fcol = texture(tex,texc);
    fragcolor = vec4(fcol.rgb*bright,fcol.a*alpha);
}
