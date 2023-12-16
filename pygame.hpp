#ifndef PYGAME_HPP
#define PYGAME_HPP
#include"include.hpp"
#include"3dgeometry.hpp"
#include"errs.hpp"
#include"gsdl.hpp"
#include"color.hpp"
#include"postp.hpp"
namespace pygame{
    inline Chlib chlib;
    inline void setup_template_0(Window& w){
        w.gl_call(glPixelStorei,GL_UNPACK_ALIGNMENT,1);
        w.gl_call(glEnable,GL_BLEND);
        w.gl_call(glBlendFunc,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        w.gl_call(glEnable,GL_CULL_FACE);
    }
    inline void setup_template_0_3D(Window& w){
        setup_template_0(w);
        w.gl_call(glEnable,GL_DEPTH_TEST);
    }
    //WARNING: Overwrites some OpenGL parameters!(Namely: -Depth Test, +Blend, *BlendFunc: srcA,1-srcA)
    //WARNING: Requires glClearColor to be set!
    void draw_made_with_glpy(Window& win,float insecs=1.625f,float staysecs=0.875f,float outsecs=1.625f);
    inline std::u8string dumppos(const glm::vec3& pos){
        return u8'('+cppp::to_u8string(pos.x)+u8','+cppp::to_u8string(pos.y)+u8','+cppp::to_u8string(pos.z)+u8')';
    }
}
#endif//PYGAME_HPP
