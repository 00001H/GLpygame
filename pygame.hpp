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
    inline namespace constants{
        constexpr float SW = 1920.0f;
        constexpr float SH = 1080.0f;
        constexpr float HSW = SW/2.0f;
        constexpr float HSH = SH/2.0f;
        constexpr float half_pi = glm::half_pi<float>();
        constexpr float pi = std::numbers::pi_v<float>;
        //https://xkcd.com/1292
        constexpr float pau = static_cast<float>(3.141592653589793l*1.5l);
        constexpr float tau = glm::two_pi<float>();
        const Point SCRCNTR = {HSW,HSH};
        const glm::vec2 SCRDIMS = {SW,SH};
        using namespace ::pygame::color::colcon;
    }
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
