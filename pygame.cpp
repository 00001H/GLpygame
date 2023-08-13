#define STB_IMAGE_IMPLEMENTATION
#define GLAD_GL_IMPLEMENTATION
#include"pygame.hpp"
namespace pygame{
    void draw_made_with_glpy(display::Window& win,float insecs,float staysecs,float outsecs){
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
}
