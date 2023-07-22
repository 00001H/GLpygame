#ifndef PYGAME_HPP
#define PYGAME_HPP
#define GLAD_GL_IMPLEMENTATION
#include<glad/glad.h>
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
        drawInit();
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
        prTexture tex = prLoadTexture2D("rsrc/glpy.png");
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
        const float size = 3.5;
        Point middle((1920.0-tex->getWidth()*size)/2.0,(1080.0-tex->getHeight()*size)/2.0);
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
            tex.alpha = visibility;
            pygame::blit(tex,middle,size);
            win.swap_buffers();
            if(frame>(inframes+stayframes+outframes))break;
            clok.tick(FPS);
            frame++;
        }
    }
}
#endif//PYGAME_HPP
