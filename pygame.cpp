#define STB_IMAGE_IMPLEMENTATION
#define GLAD_GL_IMPLEMENTATION
#include"pygame.hpp"
namespace pygame{
    void draw_made_with_glpy(Window& w,float insecs,float staysecs,float outsecs){
        const float FPS=60.00;
        sTexture _tex{load_texture(w,u8"rsrc/glpy.png"s)};
        zTexture tex{*_tex};
        #define sec2frm(sec) (glm::round((sec)*FPS))
        float inframes = sec2frm(insecs);
        float stayframes = sec2frm(staysecs);
        float outframes = sec2frm(outsecs);
        #undef sec2frm
        float frame=0;
        w.gl_call(glDisable,GL_DEPTH_TEST);
        w.gl_call(glEnable,GL_BLEND);
        w.gl_call(glBlendFunc,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        float visibility;
        time::Clock clok;
        constexpr float size = 3.5f;
        Point middle((1920.0-tex.width()*size)/2.0,(1080.0-tex.height()*size)/2.0);
        while(!w.should_close()){
            glfwPollEvents();
            w.eventqueue.get();
            w.gl_call(glClear,GL_COLOR_BUFFER_BIT);
            if(0<=frame&&frame<=inframes){
                visibility = frame/inframes;
            }else if(inframes<frame&&frame<=(inframes+stayframes)){
                visibility = 1;
            }else{
                visibility = 1-((frame-stayframes-inframes)/outframes);
            }
            tex.alpha() = visibility;
            w.blit(tex,middle,size);
            w.swap_buffers();
            if(frame>(inframes+stayframes+outframes))break;
            clok.tick(FPS);
            frame++;
        }
    }
}
