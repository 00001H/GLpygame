#ifndef PYGAME_HPP
#define PYGAME_HPP
#include"glad/glad.h"
#include"3dgeometry.hpp"
#include"errs.hpp"
#include"glfwPygame.hpp"
#include"gsdl.hpp"
#include"color.hpp"
#include"postp.hpp"
namespace pygame{
    namespace{
        bool _is_init = false;
        bool _is_gl_init = false;
    }
    void init(){
        if(!_is_init)
            display::init();
        _is_init = true;
    }
    void setupTemplate0(){
        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }
    void quit(){
        if(_is_init)
            display::quit();
        _is_init = false;
    }
    void drawInit(){
        if(!_is_gl_init){
            gllInit();
            ppInit();
        }
        _is_gl_init = true;
    }
    //WARNING: Overwrites some OpenGL parameters!
    //WARNING: Requires glClearColor to be set!
    void draw_made_with_glpy(display::Window& win,float insecs=1.625,float staysecs=0.875,float outsecs=1.625){
        stbi_set_flip_vertically_on_load(true);
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
        while(!win.shouldClose()){
            glfwPollEvents();
            win.eventqueue->get();
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
            win.swapBuffers();
            if(frame>(inframes+stayframes+outframes))break;
            clok.tick(FPS);
            frame++;
        }
    }
}
#endif//PYGAME_HPP
