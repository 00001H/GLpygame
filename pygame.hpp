#ifndef PYGAME_HPP
#define PYGAME_HPP
#ifndef PYGAME_NO3D
#include"3dgeometry.hpp"
#endif
#include"errs.hpp"
#include"glfwPygame.hpp"
#include"gsdl.hpp"
#include"color.hpp"
#include"postp.hpp"
namespace pygame{
    inline namespace constants{
        constexpr float SW = 1920.0f;
        constexpr float SH = 1080.0f;
        constexpr float HSW = SW/2.0f;
        constexpr float HSH = SH/2.0f;
        constexpr float half_pi = glm::half_pi<float>();
        constexpr float pi = glm::pi<float>();
        //https://xkcd.com/1292
        constexpr float pau = static_cast<float>(glm::pi<long double>()*1.5l);
        constexpr float tau = glm::two_pi<float>();
        const Point SCRCNTR = {HSW,HSH};
        const glm::vec2 SCRDIMS = {SW,SH};
        using namespace pygame::color::colcon;
    }
    namespace{
        bool _is_gl_init = false;
    }
    using display::init;
    void setupTemplate0(){
        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        drawInit();
    }
    void setupTemplate0_3D(){
        setupTemplate0();
        glEnable(GL_DEPTH_TEST);
    }
    using display::quit;
    void drawInit(){
        if(!_is_gl_init){
            gllInit();
            ppInit();
        }
        _is_gl_init = true;
    }
    //WARNING: Overwrites some OpenGL parameters!(Namely: -Depth Test, +Blend, *BlendFunc: srcA,1-srcA)
    //WARNING: Requires glClearColor to be set!
    void draw_made_with_glpy(display::Window& win,float insecs=1.625,float staysecs=0.875,float outsecs=1.625){
        const float FPS=60.00;
        sTexture _tex{loadTexture2D(u8"rsrc/glpy.png"s)};
        zTexture tex{*_tex};
        #define sec2frm(sec) (glm::round((sec)*FPS))
        float inframes = sec2frm(insecs);
        float stayframes = sec2frm(staysecs);
        float outframes = sec2frm(outsecs);
        #undef sec2frm
        float frame=0;
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        float visibility;
        time::Clock clok;
        constexpr float size = 3.5f;
        Point middle((1920.0-tex.width()*size)/2.0,(1080.0-tex.height()*size)/2.0);
        while(!win.should_close()){
            glfwPollEvents();
            win.eventqueue.get();
            glClear(GL_COLOR_BUFFER_BIT);
            if(0<=frame&&frame<=inframes){
                visibility = frame/inframes;
            }else if(inframes<frame&&frame<=(inframes+stayframes)){
                visibility = 1;
            }else{
                visibility = 1-((frame-stayframes-inframes)/outframes);
            }
            tex.alpha() = visibility;
            pygame::blit(tex,middle,size);
            win.swap_buffers();
            if(frame>(inframes+stayframes+outframes))break;
            clok.tick(FPS);
            frame++;
        }
    }
    std::u8string dumppos(const glm::vec3& pos){
        return u8'('+cppp::to_u8string(pos.x)+u8','+cppp::to_u8string(pos.y)+u8','+cppp::to_u8string(pos.z)+u8')';
    }
}
#endif//PYGAME_HPP
